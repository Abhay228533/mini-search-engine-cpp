# Mini Search Engine in C++

A C++20-based mini search engine that indexes text documents, builds an inverted index, and retrieves relevant documents using BM25 ranking. The project includes a browser-based search interface, unit tests, and a benchmarking executable.

## Features

- **Inverted Index:** Maps terms to documents containing them.
- **Text Processing:** Processes document text for searching.
- **BM25 Ranking:** Scores and ranks documents by query relevance.
- **Trie-Based Autocomplete:** Supports prefix-based suggestions.
- **Query Processing:** Parses user search queries.
- **Index Persistence:** Supports saving and loading index data.
- **Web Interface:** Search documents from a browser and inspect result previews and document details.
- **Automated Tests:** Tests core search functionality and persistence.
- **Benchmarking:** Measures indexing performance and search latency.

## Tech Stack

- C++20
- CMake
- HTTP library: cpp-httplib
- JSON library: nlohmann/json
- HTML, CSS, and JavaScript for the web interface

## Project Structure

```text
mini_search_engine_optimized/
├── benchmarks/    # Performance benchmark
├── data/
│   ├── raw/       # Raw input documents
│   ├── sample/    # Sample search corpus
│   └── realistic/ # Technical text documents
├── docs/          # Architecture and complexity notes
├── include/       # Header files
├── scripts/       # Dataset generation and processing
├── src/           # Search engine implementation
├── tests/         # Unit tests
├── web/           # Browser-based interface
├── CMakeLists.txt
└── README.md
```

## Prerequisites

- A C++ compiler supporting C++20
- CMake 3.16 or newer
- The cpp-httplib and nlohmann/json CMake packages for the web target

## Build

Run these commands from the project root:

```bash
cmake -S . -B build
cmake --build build
```

## Run the Search Engine

Run the terminal application:

```bash
./build/search_engine
```

## Run the Web Interface

Start the web server from the project root:

```bash
./build/search_web
```

Open this address in your browser:

http://localhost:8080

Keep the terminal running while using the web interface. Stop the server with `Ctrl+C`.

## Run Tests

Run the test executable directly:

```bash
./build/unit_tests
```

Or use CTest:

```bash
ctest --test-dir build --output-on-failure
```

## Run the Benchmark

```bash
./build/benchmark_search
```

The benchmark uses a synthetic corpus. Results depend on the machine, compiler, build configuration, and workload.

### Example Benchmark Results

One run on a MacBook Air M2 reported the following for 10,000 synthetic documents:

| Metric                |                 Result |
| --------------------- | ---------------------: |
| Indexing time         |             315.765 ms |
| Average query latency |               7.936 ms |
| Median query latency  |               7.673 ms |
| P95 query latency     |               9.324 ms |
| P99 query latency     |               9.695 ms |
| Throughput            | 126.002 queries/second |

These figures are measurements from one run, not guaranteed performance.

## Current Scope

This project is intended for learning and demonstrating information retrieval concepts, document indexing, ranking, C++ development, testing, and performance measurement. The benchmark corpus is synthetic, while the web interface can load the project's configured text-document dataset.

## Future Improvements

- Add more automated test cases and edge-case coverage.
- Evaluate ranking quality with a relevance-labelled dataset.
- Improve documentation and reproducibility of benchmarks.
- Add larger and more diverse document collections.
