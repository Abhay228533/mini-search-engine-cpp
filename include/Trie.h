#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <unordered_map>
#include <vector>

class TrieNode {
public:
    std::unordered_map<char, TrieNode*> children;
    bool isEndOfWord = false;
    
    ~TrieNode() {
        for (auto& pair : children) {
            delete pair.second;
        }
    }
};

class Trie {
public:
    Trie();
    ~Trie();
    
    void insert(const std::string& word);
    void clear();
    bool search(const std::string& word) const;
    std::vector<std::string> getAutocompleteSuggestions(const std::string& prefix, int limit = 5) const;

private:
    TrieNode* root_;
    void collectAllWords(TrieNode* node, std::string currentPrefix, std::vector<std::string>& results, int limit) const;
};

#endif // TRIE_H