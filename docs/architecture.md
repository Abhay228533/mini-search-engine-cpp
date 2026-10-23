# Mini Search Engine - System Architecture

The Mini Search Engine is designed using modular object-oriented principles in C++20, decoupling text normalization, index management, boolean query processing, ranking, and storage.

## Core Modules
- **Document & TextProcessor**: Cleans raw text, handles case-folding, punctuation removal, and tokenization.
- **InvertedIndex**: Maps string terms to sorted `Posting` lists containing `documentId` and `termFrequency`.
- **Trie**: Prefix-tree data structure powering lightning-fast autocomplete suggestions.
- **QueryParser**: Evaluates boolean operations (`AND`, `OR`, `NOT`) via linear-time $O(N + M)$ sorted list intersections.
- **Ranker**: Implements TF-IDF scoring and relevance ranking.
- **IndexStorage**: Handles binary serialization and deserialization of the inverted index to/from disk.
- **SearchEngine (Facade)**: Unifies all components behind a clean, high-level API.