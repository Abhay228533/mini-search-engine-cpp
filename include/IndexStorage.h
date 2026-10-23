#ifndef INDEX_STORAGE_H
#define INDEX_STORAGE_H

#include "Document.h"
#include "InvertedIndex.h"
#include <string>
#include <vector>

class IndexStorage {
public:
    // Saves the inverted index and document metadata in a versioned binary format.
    static bool saveIndex(const InvertedIndex& index,
                          const std::vector<Document>& documents,
                          const std::string& filepath);

    // Loads into temporary objects first; callers' data is unchanged on failure.
    static bool loadIndex(InvertedIndex& index,
                          std::vector<Document>& documents,
                          const std::string& filepath);
};

#endif // INDEX_STORAGE_H
