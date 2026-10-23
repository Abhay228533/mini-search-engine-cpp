
#include "Ranker.h"
#include "SearchEngine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
using Microseconds = std::chrono::duration<double, std::micro>;

struct BenchmarkResult {
    int documentCount = 0;
    double indexingMs = 0.0;
    double indexingDocsPerSecond = 0.0;
    double averageUs = 0.0;
    double medianUs = 0.0;
    double p95Us = 0.0;
    double p99Us = 0.0;
    double queriesPerSecond = 0.0;
    std::size_t resultsConsumed = 0;
};

double percentile(const std::vector<double>& sorted, double p) {
    if (sorted.empty()) {
        return 0.0;
    }

    const auto index = static_cast<std::size_t>(
        std::ceil(p * sorted.size())
    );

    return sorted[std::max<std::size_t>(1, index) - 1];
}

BenchmarkResult runBenchmark(int documentCount) {
    SearchEngine engine;
    BenchmarkResult result;
    result.documentCount = documentCount;

    // Generate a deterministic synthetic corpus and measure indexing.
    const auto buildStart = Clock::now();

    for (int i = 1; i <= documentCount; ++i) {
        const std::string content =
            "machine learning algorithms search engine "
            "indexing information retrieval ranking model "
            "document data structures query processing "
            "machine learning document " + std::to_string(i);

        engine.addDocument({
            i,
            "Document " + std::to_string(i),
            "synthetic/doc" + std::to_string(i) + ".txt",
            0
        }, content);
    }

    const auto buildEnd = Clock::now();

    result.indexingMs =
        std::chrono::duration<double, std::milli>(
            buildEnd - buildStart
        ).count();

    if (result.indexingMs > 0.0) {
        result.indexingDocsPerSecond =
            documentCount * 1000.0 / result.indexingMs;
    }

    const std::vector<std::string> queries = {
        "machine learning indexing",
        "search engine",
        "algorithms model",
        "information retrieval",
        "document processing"
    };

    constexpr int topK = 5;
    constexpr int warmupIterations = 200;
    constexpr int measuredQueries = 5000;

    // Warm up the search engine before collecting measurements.
    for (int i = 0; i < warmupIterations; ++i) {
        const auto& query =
            queries[static_cast<std::size_t>(i) % queries.size()];

        const auto results = engine.search(query, topK);
        result.resultsConsumed += results.size();
    }

    std::vector<double> latencies;
    latencies.reserve(measuredQueries);

    double totalMeasuredUs = 0.0;

    // Reset internal Ranker profiling after warm-up.
    resetRankProfile();

    // Measure individual query latency.
    for (int i = 0; i < measuredQueries; ++i) {
        const auto& query =
            queries[static_cast<std::size_t>(i) % queries.size()];

        const auto start = Clock::now();
        const auto results = engine.search(query, topK);
        const auto end = Clock::now();

        const double elapsedUs =
            Microseconds(end - start).count();

        latencies.push_back(elapsedUs);
        totalMeasuredUs += elapsedUs;
        result.resultsConsumed += results.size();
    }

    // Print the average time spent inside Ranker stages.
    std::cout << "\n--- Ranker Profiling: "
              << documentCount << " documents ---\n";
    printRankProfile();
    std::cout << '\n';

    std::sort(latencies.begin(), latencies.end());

    result.averageUs = totalMeasuredUs / measuredQueries;
    result.medianUs = percentile(latencies, 0.50);
    result.p95Us = percentile(latencies, 0.95);
    result.p99Us = percentile(latencies, 0.99);

    if (totalMeasuredUs > 0.0) {
        result.queriesPerSecond =
            measuredQueries * 1'000'000.0 / totalMeasuredUs;
    }

    return result;
}

int main() {
    std::cout << "=== Mini Search Engine Benchmark ===\n";
    std::cout << "Synthetic corpus; results are machine-dependent.\n";
    std::cout << "Latency includes per-query timer overhead.\n";
    std::cout << "Ranker profiling excludes tokenization and candidate collection.\n\n";

    const std::vector<int> datasetSizes = {
        100, 1000, 10000
    };

    std::cout << std::fixed << std::setprecision(3);

    for (const int size : datasetSizes) {
        const auto r = runBenchmark(size);

        std::cout << "----------------------------------------\n";
        std::cout << "Documents: " << r.documentCount << '\n';
        std::cout << "Indexing time: "
                  << r.indexingMs << " ms\n";
        std::cout << "Indexing throughput: "
                  << r.indexingDocsPerSecond << " docs/sec\n";
        std::cout << "Measured queries: 5000\n";
        std::cout << "Top-K: 5\n";
        std::cout << "Average latency: "
                  << r.averageUs << " us/query\n";
        std::cout << "Median latency: "
                  << r.medianUs << " us/query\n";
        std::cout << "P95 latency: "
                  << r.p95Us << " us/query\n";
        std::cout << "P99 latency: "
                  << r.p99Us << " us/query\n";
        std::cout << "Throughput: "
                  << r.queriesPerSecond << " queries/sec\n";
        std::cout << "Results consumed (including warm-up): "
                  << r.resultsConsumed << '\n';
    }

    std::cout << "\nBenchmark complete.\n";
    return 0;
}
