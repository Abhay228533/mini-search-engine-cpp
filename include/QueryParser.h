#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <string>
#include <vector>
#include "InvertedIndex.h"

class QueryParser {
public:
    // Performs an AND operation between two posting lists
    static std::vector<int> intersectPostings(const std::vector<Posting>& list1, const std::vector<Posting>& list2);

    // Performs an OR operation between two posting lists
    static std::vector<int> unionPostings(const std::vector<Posting>& list1, const std::vector<Posting>& list2);

    // Performs a NOT operation (list1 EXCEPT list2)
    static std::vector<int> excludePostings(const std::vector<Posting>& list1, const std::vector<Posting>& list2);
};

#endif // QUERY_PARSER_H