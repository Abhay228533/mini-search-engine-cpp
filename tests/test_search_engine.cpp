
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "TextProcessor.h"
#include "Trie.h"
#include "SearchEngine.h"

// ---------------------------------------------------------
// Test 1: Text Processor
// ---------------------------------------------------------
void testTextProcessor() {
    std::cout << "Running testTextProcessor...\n";

    const auto tokens =
        TextProcessor::tokenize(
            "  Machine Learning & AI-Driven Search!  ");

    assert(tokens.size() == 5);
    assert(tokens[0] == "machine");
    assert(tokens[1] == "learning");

    std::cout << "[PASSED] TextProcessor test\n";
}

// ---------------------------------------------------------
// Test 2: Trie Autocomplete
// ---------------------------------------------------------
void testTrieAutocomplete() {
    std::cout << "Running testTrieAutocomplete...\n";

    Trie trie;
    trie.insert("algorithm");
    trie.insert("algebra");
    trie.insert("arch");

    const auto suggestions =
        trie.getAutocompleteSuggestions("alg", 5);

    assert(suggestions.size() == 2);
    assert(trie.search("algorithm"));
    assert(!trie.search("algo"));

    std::cout << "[PASSED] Trie Autocomplete test\n";
}

// ---------------------------------------------------------
// Test 3: Search Engine Facade and Ranking
// ---------------------------------------------------------
void testSearchEngineFacade() {
    std::cout << "Running testSearchEngineFacade...\n";

    SearchEngine engine;

    engine.addDocument(
        {1, "Deep Learning Guide", "doc1.txt", 0},
        "deep learning and neural networks");

    engine.addDocument(
        {2, "Search Engine Intro", "doc2.txt", 0},
        "search engine indexing and retrieval");

    // Basic search behavior.
    const auto results = engine.search("deep learning", 5);

    assert(!results.empty());
    assert(results[0].documentId == 1);

    // Missing terms and invalid topK.
    assert(engine.search("missingword", 5).empty());
    assert(engine.search("deep", 0).empty());
    assert(engine.search("deep", -1).empty());

    // A topK value of 1 must return at most one result.
    const auto topOne = engine.search("deep learning", 1);

    assert(topOne.size() == 1);
    assert(topOne[0].documentId == 1);

    // Returned results must be ordered by descending score.
    const auto rankedResults = engine.search("search", 5);

    assert(!rankedResults.empty());

    for (std::size_t i = 1; i < rankedResults.size(); ++i) {
        assert(
            rankedResults[i - 1].score >=
            rankedResults[i].score);
    }

    // Equal scores must be ordered by ascending document ID.
    // Insert ID 20 first to ensure insertion order does not decide ties.
    SearchEngine tieEngine;

    tieEngine.addDocument(
        {20, "Document Twenty", "doc20.txt", 0},
        "common term");

    tieEngine.addDocument(
        {10, "Document Ten", "doc10.txt", 0},
        "common term");

    const auto tieResults = tieEngine.search("common", 5);

    assert(tieResults.size() == 2);
    assert(tieResults[0].score == tieResults[1].score);
    assert(tieResults[0].documentId == 10);
    assert(tieResults[1].documentId == 20);

    // topK must limit results even when several documents match.
    SearchEngine topKEngine;

    topKEngine.addDocument(
        {1, "First Document", "first.txt", 0},
        "shared search term");

    topKEngine.addDocument(
        {2, "Second Document", "second.txt", 0},
        "shared search term");

    topKEngine.addDocument(
        {3, "Third Document", "third.txt", 0},
        "shared search term");

    const auto limitedResults = topKEngine.search("shared", 2);

    assert(limitedResults.size() == 2);

    std::cout << "[PASSED] Search Engine Facade and Ranking tests\n";
}

// ---------------------------------------------------------
// Test 4: Index Persistence
// ---------------------------------------------------------
void testPersistenceRoundTrip() {
    std::cout << "Running testPersistenceRoundTrip...\n";

    const std::string path = "test_index_roundtrip.bin";

    SearchEngine original;

    original.addDocument(
        {7, "C++ Search Guide", "guide.txt", 0},
        "cpp search engine indexing");

    original.addDocument(
        {9, "Machine Learning", "ml.txt", 0},
        "machine learning models and search");

    const auto before = original.search("search", 5);

    assert(!before.empty());
    assert(original.saveIndex(path));

    SearchEngine restored;

    assert(restored.loadIndex(path));

    const auto after = restored.search("search", 5);

    assert(after.size() == before.size());
    assert(after[0].documentId == before[0].documentId);
    assert(after[0].title == before[0].title);

    // Autocomplete must work after restoring the index.
    assert(restored.autocomplete("mach", 5).size() == 1);

    // A malformed file must fail without destroying loaded data.
    const std::string badPath = "test_index_corrupt.bin";

    {
        std::ofstream bad(badPath, std::ios::binary);
        bad << "bad";
    }

    assert(!restored.loadIndex(badPath));
    assert(!restored.search("search", 5).empty());

    std::remove(path.c_str());
    std::remove(badPath.c_str());

    std::cout << "[PASSED] Persistence round-trip test\n";
}

// ---------------------------------------------------------
// Main: Run all tests
// ---------------------------------------------------------
int main() {
    std::cout << "=== Running Mini Search Engine Test Suite ===\n\n";

    testTextProcessor();
    testTrieAutocomplete();
    testSearchEngineFacade();
    testPersistenceRoundTrip();

    std::cout << "\nAll unit tests passed successfully!\n";

    return 0;
}
