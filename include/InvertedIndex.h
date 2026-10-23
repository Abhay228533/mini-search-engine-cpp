#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <string>
#include <vector>
#include <unordered_map>

struct Posting {
    int documentId;
    int termFrequency;
};

class InvertedIndex {
public:
    void addDocument(int docId, const std::vector<std::string>& tokens);
    const std::vector<Posting>& getPostingList(const std::string& term) const;
    void printIndex() const;

    // Added methods for serialization / persistence
    const std::unordered_map<std::string, std::vector<Posting>>& getRawIndex() const { return index_; }
    void setRawIndex(const std::unordered_map<std::string, std::vector<Posting>>& index) { index_ = index; }

private:
    std::unordered_map<std::string, std::vector<Posting>> index_;
};

#endif // INVERTED_INDEX_H