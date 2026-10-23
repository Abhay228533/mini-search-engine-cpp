
#include "Ranker.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace
{
    using ProfileClock = std::chrono::steady_clock;

    struct RankProfile
    {
        double scoringUs = 0.0;
        double resultBuildUs = 0.0;
        double sortingUs = 0.0;

        std::size_t queries = 0;
    };

    RankProfile profile;

    double elapsedUs(
        ProfileClock::time_point start,
        ProfileClock::time_point end)
    {
        return std::chrono::duration<double, std::micro>(
                   end - start)
            .count();
    }

} // namespace

// Reset profiling measurements before each benchmark dataset.
void resetRankProfile()
{
    profile = RankProfile{};
}

// Print aggregated profiling measurements.
void printRankProfile()
{
    if (profile.queries == 0)
    {
        std::cout << "\n--- Ranker Profiling ---\n";
        std::cout << "No ranking profile data collected.\n";
        return;
    }

    const double count =
        static_cast<double>(profile.queries);

    std::cout << "\n--- Ranker Profiling ---\n";
    std::cout << "Profiled queries: "
              << profile.queries << '\n';

    std::cout << "BM25 scoring: "
              << profile.scoringUs / count
              << " us/query\n";

    std::cout << "Result construction: "
              << profile.resultBuildUs / count
              << " us/query\n";

    std::cout << "Top-K selection + sorting: "
              << profile.sortingUs / count
              << " us/query\n";
}

// Initialize Ranker with the cached average document length.
Ranker::Ranker(
    const InvertedIndex& index,
    const std::vector<Document>& documents,
    const std::unordered_map<int, std::size_t>& documentPositions,
    double averageDocumentLength)
    : index_(index),
      documents_(documents),
      documentPositions_(documentPositions),
      totalDocuments_(documents.size()),
      averageDocumentLength_(averageDocumentLength)
{
}

// Retained for compatibility with the existing implementation.
double Ranker::calculateTFIDF(
    int docId,
    const std::string& term) const
{
    const auto& postings = index_.getPostingList(term);

    if (postings.empty() || totalDocuments_ == 0)
    {
        return 0.0;
    }

    for (const auto& posting : postings)
    {
        if (posting.documentId == docId)
        {
            const double tf =
                static_cast<double>(posting.termFrequency);

            const double df =
                static_cast<double>(postings.size());

            const double idf = std::log(
                1.0 +
                (static_cast<double>(totalDocuments_) - df + 0.5) /
                    (df + 0.5));

            return idf * (tf * 2.2) / (tf + 1.2);
        }
    }

    return 0.0;
}

std::vector<SearchResult> Ranker::rankQuery(
    const std::vector<std::string>& queryTokens,
    const std::vector<int>& candidateDocIds,
    int topK)
{
    if (
        topK <= 0 ||
        candidateDocIds.empty() ||
        totalDocuments_ == 0 ||
        queryTokens.empty())
    {
        return {};
    }

    // Reuse the cached value instead of scanning all documents
    // for every search query.
    const double averageLength = averageDocumentLength_;

    // Standard BM25 parameters.
    constexpr double k1 = 1.2;
    constexpr double b = 0.75;

    // Initialize scores for candidate documents.
    std::unordered_map<int, double> scores;
    scores.reserve(candidateDocIds.size());

    for (int id : candidateDocIds)
    {
        scores.try_emplace(id, 0.0);
    }

    // ---------------------------------------------------------
    // 1. BM25 scoring.
    // ---------------------------------------------------------

    const auto scoringStart = ProfileClock::now();

    // Repeated query terms contribute repeatedly, preserving
    // the existing query-token behavior.
    for (const auto& term : queryTokens)
    {
        const auto& postings = index_.getPostingList(term);

        if (postings.empty())
        {
            continue;
        }

        const double df =
            static_cast<double>(postings.size());

        const double idf = std::log(
            1.0 +
            (static_cast<double>(totalDocuments_) - df + 0.5) /
                (df + 0.5));

        for (const auto& posting : postings)
        {
            auto scoreIt = scores.find(posting.documentId);

            if (scoreIt == scores.end())
            {
                continue;
            }

            const auto positionIt =
                documentPositions_.find(posting.documentId);

            if (positionIt == documentPositions_.end())
            {
                continue;
            }

            const Document& doc =
                documents_[positionIt->second];

            const double tf =
                static_cast<double>(posting.termFrequency);

            const double docLength =
                static_cast<double>(doc.tokenCount);

            const double lengthNormalization =
                averageLength > 0.0
                    ? (1.0 - b + b * docLength / averageLength)
                    : 1.0;

            const double denominator =
                tf + k1 * lengthNormalization;

            if (denominator > 0.0)
            {
                scoreIt->second +=
                    idf * (tf * (k1 + 1.0)) / denominator;
            }
        }
    }

    const auto scoringEnd = ProfileClock::now();

    profile.scoringUs += elapsedUs(
        scoringStart,
        scoringEnd);

    // Lightweight representation used for top-K selection.
    struct ScoredDocument
    {
        int documentId;
        double score;
        std::size_t documentPosition;
    };

    // ---------------------------------------------------------
    // 2. Select and sort the top-K documents.
    // ---------------------------------------------------------

    const auto sortingStart = ProfileClock::now();

    std::vector<ScoredDocument> rankedDocuments;
    rankedDocuments.reserve(scores.size());

    for (const auto& [docId, score] : scores)
    {
        const auto positionIt =
            documentPositions_.find(docId);

        if (positionIt == documentPositions_.end())
        {
            continue;
        }

        rankedDocuments.push_back({
            docId,
            score,
            positionIt->second
        });
    }

    // Higher score first; break ties by ascending document ID.
    const auto scoredComparator =
        [](const ScoredDocument& a, const ScoredDocument& b)
        {
            if (a.score != b.score)
            {
                return a.score > b.score;
            }

            return a.documentId < b.documentId;
        };

    const std::size_t k = std::min(
        rankedDocuments.size(),
        static_cast<std::size_t>(topK));

    if (k < rankedDocuments.size())
    {
        std::nth_element(
            rankedDocuments.begin(),
            rankedDocuments.begin() +
                static_cast<std::ptrdiff_t>(k),
            rankedDocuments.end(),
            scoredComparator);

        rankedDocuments.resize(k);
    }

    std::sort(
        rankedDocuments.begin(),
        rankedDocuments.end(),
        scoredComparator);

    const auto sortingEnd = ProfileClock::now();

    profile.sortingUs += elapsedUs(
        sortingStart,
        sortingEnd);

    // ---------------------------------------------------------
    // 3. Construct full results only for the selected documents.
    // ---------------------------------------------------------

    const auto resultBuildStart = ProfileClock::now();

    std::vector<SearchResult> results;
    results.reserve(rankedDocuments.size());

    for (const auto& ranked : rankedDocuments)
    {
        const Document& doc =
            documents_[ranked.documentPosition];

        results.push_back({
            ranked.documentId,
            doc.title,
            ranked.score
        });
    }

    const auto resultBuildEnd = ProfileClock::now();

    profile.resultBuildUs += elapsedUs(
        resultBuildStart,
        resultBuildEnd);

    ++profile.queries;

    return results;
}
