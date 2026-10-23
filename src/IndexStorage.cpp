#include "IndexStorage.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
constexpr char MAGIC[] = {'M', 'S', 'E', 'I', 'D', 'X', '0', '1'};
constexpr std::uint32_t FORMAT_VERSION = 1;
// Defensive limits prevent corrupt files from requesting unreasonable allocations.
constexpr std::uint64_t MAX_TERMS = 5'000'000;
constexpr std::uint64_t MAX_DOCUMENTS = 5'000'000;
constexpr std::uint64_t MAX_STRING_BYTES = 16 * 1024 * 1024;
constexpr std::uint64_t MAX_POSTINGS_PER_TERM = 5'000'000;

bool writeBytes(std::ostream& out, const void* data, std::size_t size) {
    out.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
    return static_cast<bool>(out);
}

bool readBytes(std::istream& in, void* data, std::size_t size) {
    in.read(static_cast<char*>(data), static_cast<std::streamsize>(size));
    return static_cast<bool>(in);
}

template <typename T>
bool writeValue(std::ostream& out, const T& value) {
    return writeBytes(out, &value, sizeof(value));
}

template <typename T>
bool readValue(std::istream& in, T& value) {
    return readBytes(in, &value, sizeof(value));
}

bool writeString(std::ostream& out, const std::string& value) {
    const auto size = static_cast<std::uint64_t>(value.size());
    return writeValue(out, size) && (size == 0 || writeBytes(out, value.data(), value.size()));
}

bool readString(std::istream& in, std::string& value) {
    std::uint64_t size = 0;
    if (!readValue(in, size) || size > MAX_STRING_BYTES || size > std::numeric_limits<std::size_t>::max()) {
        return false;
    }
    std::string temporary(static_cast<std::size_t>(size), '\0');
    if (size != 0 && !readBytes(in, temporary.data(), temporary.size())) {
        return false;
    }
    value = std::move(temporary);
    return true;
}
}

bool IndexStorage::saveIndex(const InvertedIndex& index,
                             const std::vector<Document>& documents,
                             const std::string& filepath) {
    std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
    if (!out) {
        std::cerr << "Error: Could not open index file for writing: " << filepath << '\n';
        return false;
    }

    if (!writeBytes(out, MAGIC, sizeof(MAGIC)) || !writeValue(out, FORMAT_VERSION)) return false;

    const auto documentCount = static_cast<std::uint64_t>(documents.size());
    if (!writeValue(out, documentCount)) return false;
    for (const auto& document : documents) {
        const auto tokenCount = static_cast<std::uint64_t>(document.tokenCount);
        if (!writeValue(out, document.id) || !writeString(out, document.title) ||
            !writeString(out, document.path) || !writeValue(out, tokenCount)) return false;
    }

    const auto& rawIndex = index.getRawIndex();
    const auto termCount = static_cast<std::uint64_t>(rawIndex.size());
    if (!writeValue(out, termCount)) return false;
    for (const auto& [term, postings] : rawIndex) {
        const auto postingCount = static_cast<std::uint64_t>(postings.size());
        if (!writeString(out, term) || !writeValue(out, postingCount)) return false;
        for (const auto& posting : postings) {
            if (!writeValue(out, posting.documentId) || !writeValue(out, posting.termFrequency)) return false;
        }
    }

    out.flush();
    return static_cast<bool>(out);
}

bool IndexStorage::loadIndex(InvertedIndex& index,
                             std::vector<Document>& documents,
                             const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in) {
        std::cerr << "Error: Could not open index file for reading: " << filepath << '\n';
        return false;
    }

    char magic[sizeof(MAGIC)]{};
    std::uint32_t version = 0;
    if (!readBytes(in, magic, sizeof(magic)) || magic[0] != MAGIC[0] || magic[1] != MAGIC[1] ||
        magic[2] != MAGIC[2] || magic[3] != MAGIC[3] || magic[4] != MAGIC[4] ||
        magic[5] != MAGIC[5] || magic[6] != MAGIC[6] || magic[7] != MAGIC[7] ||
        !readValue(in, version) || version != FORMAT_VERSION) {
        std::cerr << "Error: Invalid or unsupported index file format: " << filepath << '\n';
        return false;
    }

    std::vector<Document> loadedDocuments;
    InvertedIndex loadedIndex;
    std::uint64_t documentCount = 0;
    if (!readValue(in, documentCount) || documentCount > MAX_DOCUMENTS) return false;
    loadedDocuments.reserve(static_cast<std::size_t>(documentCount));
    for (std::uint64_t i = 0; i < documentCount; ++i) {
        Document document{};
        std::uint64_t tokenCount = 0;
        if (!readValue(in, document.id) || !readString(in, document.title) ||
            !readString(in, document.path) || !readValue(in, tokenCount) ||
            tokenCount > std::numeric_limits<std::size_t>::max()) return false;
        document.tokenCount = static_cast<std::size_t>(tokenCount);
        loadedDocuments.push_back(std::move(document));
    }

    std::uint64_t termCount = 0;
    if (!readValue(in, termCount) || termCount > MAX_TERMS) return false;
    std::unordered_map<std::string, std::vector<Posting>> rawIndex;
    rawIndex.reserve(static_cast<std::size_t>(termCount));
    for (std::uint64_t i = 0; i < termCount; ++i) {
        std::string term;
        std::uint64_t postingCount = 0;
        if (!readString(in, term) || term.empty() || !readValue(in, postingCount) ||
            postingCount > MAX_POSTINGS_PER_TERM) return false;
        std::vector<Posting> postings;
        postings.reserve(static_cast<std::size_t>(postingCount));
        int previousId = std::numeric_limits<int>::min();
        for (std::uint64_t j = 0; j < postingCount; ++j) {
            Posting posting{};
            if (!readValue(in, posting.documentId) || !readValue(in, posting.termFrequency) ||
                posting.termFrequency <= 0 || (j > 0 && posting.documentId <= previousId)) return false;
            previousId = posting.documentId;
            postings.push_back(posting);
        }
        if (!rawIndex.emplace(std::move(term), std::move(postings)).second) return false;
    }

    // Reject trailing garbage and truncated/partially written files.
    if (in.peek() != std::char_traits<char>::eof()) return false;
    loadedIndex.setRawIndex(rawIndex);
    index = std::move(loadedIndex);
    documents = std::move(loadedDocuments);
    return true;
}
