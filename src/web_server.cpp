
#include "SearchEngine.h"
#include "httplib.h"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

// --------------------------------------------------
// Load documents into the search engine
// --------------------------------------------------

void loadDatasetIntoEngine(
    SearchEngine &engine,
    const std::string &directoryPath,
    int firstFallbackId = 1,
    bool useFilenameIds = true)
{
    if (!fs::exists(directoryPath) ||
        !fs::is_directory(directoryPath))
    {
        throw std::runtime_error(
            "Dataset directory not found: " + directoryPath);
    }

    std::vector<fs::path> files;

    for (const auto &entry : fs::directory_iterator(directoryPath))
    {
        if (entry.is_regular_file() &&
            entry.path().extension() == ".txt")
        {
            files.push_back(entry.path());
        }
    }

    // Keep loading order consistent between runs.
    std::sort(files.begin(), files.end());

    int count = 0;

    for (const auto &path : files)
    {
        std::ifstream file(path);

        if (!file.is_open())
        {
            std::cerr << "Skipping unreadable file: "
                      << path << '\n';
            continue;
        }

        std::string title;
        std::getline(file, title);

        if (title.empty())
        {
            title = path.stem().string();
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        // Use a unique sequential ID by default.
        int documentId = firstFallbackId + count;
        const std::string filename = path.stem().string();

        // Preserve IDs such as doc1.txt, doc2.txt for sample data.
        if (useFilenameIds &&
            filename.rfind("doc", 0) == 0 &&
            filename.size() > 3)
        {
            try
            {
                std::size_t parsedLength = 0;
                const int parsedId =
                    std::stoi(filename.substr(3), &parsedLength);

                if (parsedLength == filename.size() - 3 &&
                    parsedId > 0)
                {
                    documentId = parsedId;
                }
            }
            catch (const std::exception &)
            {
                // Keep the sequential fallback ID.
            }
        }

        engine.addDocument(
            {documentId, title, path.string(), 0},
            buffer.str());

        ++count;
    }

    std::cout << "Loaded " << count
              << " documents from "
              << directoryPath << ".\n";
}

// --------------------------------------------------
// Read a preview around a matching query term
// --------------------------------------------------
std::string readDocumentPreview(
    const std::string &path,
    const std::string &query,
    std::size_t maxLength = 240)
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        return "Preview is not available.";
    }

    std::string line;
    std::string content;

    // The first line contains the document title.
    std::getline(file, line);

    // Read the complete body so matches later in the document
    // can be included in the preview.
    while (std::getline(file, line))
    {

        if (!content.empty())
        {
            content += ' ';
        }
        content += line;
    }

    // Normalize whitespace.
    std::string cleaned;
    cleaned.reserve(content.size());
    bool previousWasSpace = false;

    for (unsigned char ch : content)
    {
        if (std::isspace(ch))
        {
            if (!previousWasSpace && !cleaned.empty())
            {
                cleaned += ' ';
            }
            previousWasSpace = true;
        }
        else
        {
            cleaned += static_cast<char>(ch);
            previousWasSpace = false;
        }
    }

    while (!cleaned.empty() && cleaned.back() == ' ')
    {
        cleaned.pop_back();
    }

    if (cleaned.empty())
    {
        return "No preview text is available.";
    }

    if (maxLength == 0)
    {
        return "";
    }

    // Make a lowercase copy for case-insensitive matching.
    std::string lowerContent = cleaned;
    std::string lowerQuery = query;

    for (char &ch : lowerContent)
    {
        ch = static_cast<char>(
            std::tolower(static_cast<unsigned char>(ch)));
    }

    for (char &ch : lowerQuery)
    {
        ch = static_cast<char>(
            std::tolower(static_cast<unsigned char>(ch)));
    }

    // Find the earliest occurrence of any query term.
    std::size_t matchPosition = std::string::npos;
    std::istringstream queryStream(lowerQuery);
    std::string term;

    while (queryStream >> term)
    {
        const std::size_t position = lowerContent.find(term);

        if (position != std::string::npos &&
            (matchPosition == std::string::npos ||
             position < matchPosition))
        {
            matchPosition = position;
        }
    }

    // If the body is short enough, return it all.
    if (cleaned.size() <= maxLength)
    {
        std::cerr << "DEBUG: " << cleaned << std::endl;
        return cleaned;
    }

    // If a query term was found, place the match roughly one-third
    // of the way into the excerpt. Otherwise, use the beginning.
    std::size_t start = 0;

    if (matchPosition != std::string::npos &&
        matchPosition > maxLength / 3)
    {
        start = matchPosition - maxLength / 3;
    }

    if (start + maxLength > cleaned.size())
    {
        start = cleaned.size() - maxLength;
    }

    // Avoid starting in the middle of a word.
    if (start > 0)
    {
        const std::size_t nextSpace = cleaned.find(' ', start);
        if (nextSpace != std::string::npos &&
            nextSpace < start + 20)
        {
            start = nextSpace + 1;
        }
    }

    std::size_t end = std::min(start + maxLength, cleaned.size());

    // Avoid ending in the middle of a word when possible.
    if (end < cleaned.size())
    {
        const std::size_t lastSpace = cleaned.rfind(' ', end);
        if (lastSpace != std::string::npos &&
            lastSpace > start &&
            lastSpace - start > maxLength / 2)
        {
            end = lastSpace;
        }
    }

    std::string preview = cleaned.substr(start, end - start);

    if (start > 0)
    {
        preview = "..." + preview;
    }

    if (end < cleaned.size())
    {
        preview += "...";
    }

    return preview;
}

int main()
{
    try
    {
        SearchEngine engine;

        // Run the executable from the project root.
        loadDatasetIntoEngine(engine, "data/sample", 1, true);
        loadDatasetIntoEngine(engine, "data/realistic", 102, true);
        httplib::Server server;

        if (!server.set_mount_point("/", "./web"))
        {
            std::cerr
                << "Could not serve ./web. "
                << "Check that web/index.html exists.\n";
            return 1;
        }

        // --------------------------------------------------
        // Search API
        // GET /api/search?q=machine%20learning
        // --------------------------------------------------
        server.Get(
            "/api/search",
            [&](const httplib::Request &req,
                httplib::Response &res)
            {
                const std::string query =
                    req.has_param("q")
                        ? req.get_param_value("q")
                        : "";

                json response;
                response["query"] = query;
                response["results"] = json::array();

                if (!query.empty())
                {
                    for (const auto &result : engine.search(query, 200))
                    {
                        json item = {
                            {"id", result.documentId},
                            {"title", result.title},
                            {"score", result.score}};

                        const Document *doc =
                            engine.getDocumentById(result.documentId);

                        if (doc != nullptr)
                        {
                            item["id"] = doc->id;
                            item["title"] = doc->title;
                            item["path"] = doc->path;
                            item["tokenCount"] = doc->tokenCount;
                            item["preview"] =
                                readDocumentPreview(doc->path, query, 240);
                        }
                        else
                        {
                            item["preview"] =
                                "Document information is unavailable.";
                        }

                        response["results"].push_back(item);
                    }
                }

                res.set_header("Cache-Control", "no-store");
                res.set_content(
                    response.dump(2),
                    "application/json; charset=utf-8");
            });

        // --------------------------------------------------
        // Autocomplete API
        // GET /api/autocomplete?q=mach
        // --------------------------------------------------
        server.Get(
            "/api/autocomplete",
            [&](const httplib::Request &req,
                httplib::Response &res)
            {
                const std::string prefix =
                    req.has_param("q")
                        ? req.get_param_value("q")
                        : "";

                json response;
                response["suggestions"] = json::array();

                if (!prefix.empty())
                {
                    for (const auto &suggestion :
                         engine.autocomplete(prefix, 8))
                    {
                        response["suggestions"].push_back(suggestion);
                    }
                }

                res.set_header("Cache-Control", "no-store");
                res.set_content(
                    response.dump(),
                    "application/json; charset=utf-8");
            });

        // --------------------------------------------------
        // Document details API
        // GET /api/document?id=30
        // --------------------------------------------------
        server.Get(
            "/api/document",
            [&](const httplib::Request &req,
                httplib::Response &res)
            {
                if (!req.has_param("id"))
                {
                    res.status = 400;
                    res.set_content(
                        R"({"error":"Missing document id"})",
                        "application/json");
                    return;
                }

                try
                {
                    const std::string idText =
                        req.get_param_value("id");

                    std::size_t parsedLength = 0;
                    const int id = std::stoi(idText, &parsedLength);

                    if (parsedLength != idText.size() || id <= 0)
                    {
                        res.status = 400;
                        res.set_content(
                            R"({"error":"Invalid document id"})",
                            "application/json");
                        return;
                    }

                    const Document *doc = engine.getDocumentById(id);

                    if (doc == nullptr)
                    {
                        res.status = 404;
                        res.set_content(
                            R"({"error":"Document not found"})",
                            "application/json");
                        return;
                    }

                    json response = {
                        {"id", doc->id},
                        {"title", doc->title},
                        {"path", doc->path},
                        {"tokenCount", doc->tokenCount},
                        {"preview", readDocumentPreview(doc->path, "", 500)}};

                    res.set_header("Cache-Control", "no-store");
                    res.set_content(
                        response.dump(2),
                        "application/json; charset=utf-8");
                }
                catch (const std::exception &)
                {
                    res.status = 400;
                    res.set_content(
                        R"({"error":"Invalid document id"})",
                        "application/json");
                }
            });

        // --------------------------------------------------
        // Start web server
        // --------------------------------------------------
        std::cout
            << "\nMini Search Engine Web Server\n"
            << "--------------------------------\n"
            << "Open: http://localhost:8080\n"
            << "Press Ctrl+C to stop the server.\n\n";

        if (!server.listen("127.0.0.1", 8080))
        {
            std::cerr
                << "Could not start the server on port 8080. "
                << "Check whether another process is using it.\n";
            return 1;
        }
    }
    catch (const std::exception &error)
    {
        std::cerr << "Startup error: "
                  << error.what() << '\n';
        return 1;
    }

    return 0;
}
