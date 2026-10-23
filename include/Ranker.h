
#ifndef RANKER_H
#define RANKER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>

#include "InvertedIndex.h"
#include "Document.h"

struct SearchResult {
    int documentId;
    std::string title;
    double score;
};

class Ranker {
public:
    Ranker(
        const InvertedIndex& index,
        const std::vector<Document>& documents,
        const std::unordered_map<int, std::size_t>& documentPositions,
        double averageDocumentLength
    );

    std::vector<SearchResult> rankQuery(
        const std::vector<std::string>& queryTokens,
        const std::vector<int>& candidateDocIds,
        int topK
    );

private:
    const InvertedIndex& index_;
    const std::vector<Document>& documents_;
    const std::unordered_map<int, std::size_t>& documentPositions_;
    std::size_t totalDocuments_;
    double averageDocumentLength_;

    double calculateTFIDF(
        int docId,
        const std::string& term
    ) const;
};

void resetRankProfile();
void printRankProfile();

#endif // RANKER_H
