#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "SearchEngine.h"

namespace fs = std::filesystem;

// Helper function to read text files from the data/sample folder
void loadDatasetIntoEngine(SearchEngine& engine, const std::string& directoryPath) {
    if (!fs::exists(directoryPath)) {
        std::cout << "Directory does not exist: " << directoryPath << "\n";
        return;
    }

    int count = 0;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::ifstream file(entry.path());
            if (file.is_open()) {
                std::string title;
                std::getline(file, title); // Read first line as title

                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string content = buffer.str();

                // Add document to your search engine
                engine.addDocument({++count, title, entry.path().string(), 0}, content);
            }
        }
    }
    std::cout << "Successfully loaded " << count << " documents from disk!\n";
}

int main() {
    std::cout << "=== Mini Search Engine: Dynamic Data Loading ==-\n\n";

    SearchEngine engine;

    // 1. Load the generated files from the data/sample folder
    loadDatasetIntoEngine(engine, "../data/sample");

    // 2. Perform a search across those real files
    std::string query = "machine learning";
    auto results = engine.search(query, 5);

    std::cout << "\nTop Search Results for '" << query << "':\n";
    for (const auto& r : results) {
        std::cout << "  - [Doc ID: " << r.documentId << "] " << r.title << " (Score: " << r.score << ")\n";
    }

    return 0;
}