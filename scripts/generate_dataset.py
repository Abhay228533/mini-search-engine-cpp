#!/usr/bin/env python3
import os
import random

VOCABULARY = [
    "machine", "learning", "artificial", "intelligence", "search", "engine",
    "indexing", "algorithms", "data", "structures", "deep", "neural", "networks",
    "retrieval", "performance", "optimization", "vectors", "scoring", "ranking"
]

def generate_documents(output_dir="data/sample", num_docs=100):
    os.makedirs(output_dir, exist_ok=True)
    print(f"Generating {num_docs} sample documents in '{output_dir}'...")
    
    for i in range(1, num_docs + 1):
        # Pick 5 to 15 random words per document
        length = random.randint(5, 15)
        doc_words = [random.choice(VOCABULARY) for _ in range(length)]
        content = " ".join(doc_words)
        
        file_path = os.path.join(output_dir, f"doc{i}.txt")
        with open(file_path, "w") as f:
            f.write(f"Document Title {i}\n{content}\n")
            
    print("Dataset generation complete!")

if __name__ == "__main__":
    generate_documents()