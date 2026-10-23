
#include "SearchEngine.h"
#include "TextProcessor.h"
#include "IndexStorage.h"

#include <unordered_set>
#include <utility>

void SearchEngine::updateAverageDocumentLength()
{
    if (documents_.empty()) {
        totalTokenCount_ = 0;
        averageDocumentLength_ = 0.0;
        return;
    }

    totalTokenCount_ = 0;

    for (const auto& doc : documents_) {
        totalTokenCount_ += doc.tokenCount;
    }

    averageDocumentLength_ =
        static_cast<double>(totalTokenCount_) /
        static_cast<double>(documents_.size());
}

// Add a document to the search engine.
void SearchEngine::addDocument(
    const Document& doc,
    const std::string& content
)
{
    // Ignore duplicate document IDs.
    if (documentPositions_.find(doc.id) != documentPositions_.end()) {
        return;
    }

    auto tokens = TextProcessor::tokenize(content);

    Document stored = doc;
    stored.tokenCount = tokens.size();

    const std::size_t position = documents_.size();

    documents_.push_back(stored);
    documentPositions_.emplace(doc.id, position);

    index_.addDocument(doc.id, tokens);

    for (const auto& token : tokens) {
        trie_.insert(token);
    }

    // Update the cached statistics incrementally.
    totalTokenCount_ += stored.tokenCount;

    averageDocumentLength_ =
        static_cast<double>(totalTokenCount_) /
        static_cast<double>(documents_.size());
}

// Search documents and return the top-ranked results.
std::vector<SearchResult> SearchEngine::search(
    const std::string& query,
    int topK
)
{
    if (topK <= 0) {
        return {};
    }

    const auto queryTokens = TextProcessor::tokenize(query);

    if (queryTokens.empty() || documents_.empty()) {
        return {};
    }

    std::unordered_set<int> seen;
    std::vector<int> candidates;

    seen.reserve(128);
    candidates.reserve(128);

    for (const auto& term : queryTokens) {
        const auto& postings = index_.getPostingList(term);

        for (const auto& posting : postings) {
            if (seen.insert(posting.documentId).second) {
                candidates.push_back(posting.documentId);
            }
        }
    }

    Ranker ranker(
        index_,
        documents_,
        documentPositions_,
        averageDocumentLength_
    );

    auto results = ranker.rankQuery(
        queryTokens,
        candidates,
        topK
    );

    if (results.size() > static_cast<std::size_t>(topK)) {
        results.resize(static_cast<std::size_t>(topK));
    }

    return results;
}

std::vector<std::string> SearchEngine::autocomplete(
    const std::string& prefix,
    int limit
)
{
    return trie_.getAutocompleteSuggestions(prefix, limit);
}

const Document* SearchEngine::getDocumentById(int id) const
{
    const auto it = documentPositions_.find(id);

    if (it == documentPositions_.end()) {
        return nullptr;
    }

    return &documents_[it->second];
}

bool SearchEngine::saveIndex(
    const std::string& filepath
) const
{
    return IndexStorage::saveIndex(
        index_,
        documents_,
        filepath
    );
}

bool SearchEngine::loadIndex(
    const std::string& filepath
)
{
    InvertedIndex loadedIndex;
    std::vector<Document> loadedDocuments;

    // Preserve the current engine if loading fails.
    if (!IndexStorage::loadIndex(
            loadedIndex,
            loadedDocuments,
            filepath
        )) {
        return false;
    }

    std::unordered_map<int, std::size_t> loadedPositions;
    loadedPositions.reserve(loadedDocuments.size());

    for (std::size_t i = 0; i < loadedDocuments.size(); ++i) {
        const auto [it, inserted] =
            loadedPositions.emplace(loadedDocuments[i].id, i);

        if (!inserted) {
            return false;
        }
    }

    // Commit only after the loaded data has been validated.
    index_ = std::move(loadedIndex);
    documents_ = std::move(loadedDocuments);
    documentPositions_ = std::move(loadedPositions);

    // Recalculate cached statistics for the newly loaded index.
    updateAverageDocumentLength();

    trie_.clear();

    for (const auto& [term, postings] : index_.getRawIndex()) {
        (void)postings;
        trie_.insert(term);
    }

    return true;
}
