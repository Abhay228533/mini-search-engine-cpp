#include "Trie.h"

Trie::Trie() {
    root_ = new TrieNode();
}

Trie::~Trie() {
    delete root_;
}

void Trie::clear() {
    delete root_;
    root_ = new TrieNode();
}

void Trie::insert(const std::string& word) {
    TrieNode* current = root_;
    for (char c : word) {
        if (current->children.find(c) == current->children.end()) {
            current->children[c] = new TrieNode();
        }
        current = current->children[c];
    }
    current->isEndOfWord = true;
}

bool Trie::search(const std::string& word) const {
    TrieNode* current = root_;
    for (char c : word) {
        if (current->children.find(c) == current->children.end()) {
            return false;
        }
        current = current->children[c];
    }
    return current->isEndOfWord;
}

void Trie::collectAllWords(TrieNode* node, std::string currentPrefix, std::vector<std::string>& results, int limit) const {
    if (results.size() >= static_cast<size_t>(limit)) return;
    
    if (node->isEndOfWord) {
        results.push_back(currentPrefix);
    }

    for (const auto& pair : node->children) {
        collectAllWords(pair.second, currentPrefix + pair.first, results, limit);
    }
}

std::vector<std::string> Trie::getAutocompleteSuggestions(const std::string& prefix, int limit) const {
    TrieNode* current = root_;
    for (char c : prefix) {
        if (current->children.find(c) == current->children.end()) {
            return {}; // Prefix not found
        }
        current = current->children[c];
    }

    std::vector<std::string> suggestions;
    collectAllWords(current, prefix, suggestions, limit);
    return suggestions;
}