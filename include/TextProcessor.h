#ifndef TEXT_PROCESSOR_H
#define TEXT_PROCESSOR_H

#include <string>
#include <vector>

class TextProcessor {
public:
    static std::vector<std::string> tokenize(const std::string& text);
};

#endif // TEXT_PROCESSOR_H