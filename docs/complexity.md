# Algorithmic Complexity Analysis

| Component / Operation | Data Structure | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- |
| **Tokenization** | `std::string` / Streams | $O(L)$ *(L = doc length)* | $O(T)$ *(T = tokens)* |
| **Index Insertion** | Hash Map + Sorted Vector | $O(T \log P)$ *(P = postings)* | $O(V \cdot P)$ *(V = vocab)* |
| **Trie Insertion / Search** | Prefix Tree (Hash Map children) | $O(K)$ *(K = key length)* | $O(V \cdot K)$ |
| **Boolean Intersection** | Two-Pointer Traversal | $O(N + M)$ *(N, M list sizes)*| $O(R)$ *(R = result size)* |
| **TF-IDF Ranking** | Accumulator Map + Sorting | $O(C \log C)$ *(C = candidates)* | $O(C)$ |
| **Index Serialization** | Binary File Streams | $O(V \cdot P)$ | $O(1)$ auxiliary |