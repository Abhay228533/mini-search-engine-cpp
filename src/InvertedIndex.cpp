
#include "InvertedIndex.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>

void InvertedIndex::addDocument(
    int docId,
    const std::vector<std::string>& tokens
) {
    // Count each term once per document.
    std::unordered_map<std::string, int> termFreqs;
    termFreqs.reserve(tokens.size());

    for (const auto& token : tokens) {
        ++termFreqs[token];
    }

    for (const auto& [term, freq] : termFreqs) {
        auto& postingList = index_[term];

        // Fast path: most new document IDs arrive in increasing order.
        if (postingList.empty() ||
            postingList.back().documentId < docId) {
            postingList.push_back({docId, freq});
            continue;
        }

        // Find an existing posting or the sorted insertion position.
        auto it = std::lower_bound(
            postingList.begin(),
            postingList.end(),
            docId,
            [](const Posting& posting, int id) {
                return posting.documentId < id;
            }
        );

        if (it != postingList.end() && it->documentId == docId) {
            // Preserve existing behavior for repeated document IDs.
            it->termFrequency += freq;
        } else {
            // Keep posting lists sorted for out-of-order IDs.
            postingList.insert(it, {docId, freq});
        }
    }
}

const std::vector<Posting>& InvertedIndex::getPostingList(
    const std::string& term
) const {
    static const std::vector<Posting> emptyList;

    const auto it = index_.find(term);

    if (it != index_.end()) {
        return it->second;
    }

    return emptyList;
}

void InvertedIndex::printIndex() const {
    for (const auto& [term, postings] : index_) {
        std::cout << term << " -> ";

        for (const auto& posting : postings) {
            std::cout
                << "[Doc ID: " << posting.documentId
                << ", TF: " << posting.termFrequency << "] ";
        }

        std::cout << '\n';
    }
}
