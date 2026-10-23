#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>

struct Document {
    int id;
    std::string title;
    std::string path;
    std::size_t tokenCount;
};

#endif // DOCUMENT_H