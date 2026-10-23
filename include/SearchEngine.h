
#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>

#include "Document.h"
#include "InvertedIndex.h"
#include "Trie.h"
#include "Ranker.h"

class SearchEngine {
public:
    void addDocument(const Document& doc, const std::string& content);

    std::vector<SearchResult> search(
        const std::string& query,
        int topK = 5
    );

    std::vector<std::string> autocomplete(
        const std::string& prefix,
        int limit = 5
    );

    bool saveIndex(const std::string& filepath) const;
    bool loadIndex(const std::string& filepath);
    const Document* getDocumentById(int id) const;

private:
    InvertedIndex index_;
    Trie trie_;
    std::vector<Document> documents_;
    std::unordered_map<int, std::size_t> documentPositions_;

    // Cached document-length statistics.
    std::size_t totalTokenCount_ = 0;
    double averageDocumentLength_ = 0.0;

    void updateAverageDocumentLength();
};

#endif // SEARCH_ENGINE_H
