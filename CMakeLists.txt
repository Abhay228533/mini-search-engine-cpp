
cmake_minimum_required(VERSION 3.16)
project(MiniSearchEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

# Common library source files
set(ENGINE_SOURCES
    src/Document.cpp
    src/TextProcessor.cpp
    src/InvertedIndex.cpp
    src/Trie.cpp
    src/QueryParser.cpp
    src/Ranker.cpp
    src/IndexStorage.cpp
    src/SearchEngine.cpp
)

# Main executable
add_executable(search_engine
    ${ENGINE_SOURCES}
    src/main.cpp
)

# Unit test executable
add_executable(unit_tests
    ${ENGINE_SOURCES}
    tests/test_search_engine.cpp
)

# Register tests with CTest
enable_testing()
add_test(NAME MiniSearchEngineTests COMMAND unit_tests)

# Benchmark executable
add_executable(benchmark_search
    ${ENGINE_SOURCES}
    benchmarks/benchmark_search.cpp
)

# Browser-based web interface
find_package(httplib CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(search_web
    ${ENGINE_SOURCES}
    src/web_server.cpp
)

target_link_libraries(search_web PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
)