#include "QueryParser.h"
#include <algorithm>

std::vector<int> QueryParser::intersectPostings(const std::vector<Posting>& list1, const std::vector<Posting>& list2) {
    std::vector<int> result;
    size_t i = 0, j = 0;

    while (i < list1.size() && j < list2.size()) {
        if (list1[i].documentId == list2[j].documentId) {
            result.push_back(list1[i].documentId);
            i++;
            j++;
        } else if (list1[i].documentId < list2[j].documentId) {
            i++;
        } else {
            j++;
        }
    }
    return result;
}

std::vector<int> QueryParser::unionPostings(const std::vector<Posting>& list1, const std::vector<Posting>& list2) {
    std::vector<int> result;
    size_t i = 0, j = 0;

    while (i < list1.size() && j < list2.size()) {
        if (list1[i].documentId == list2[j].documentId) {
            result.push_back(list1[i].documentId);
            i++;
            j++;
        } else if (list1[i].documentId < list2[j].documentId) {
            result.push_back(list1[i].documentId);
            i++;
        } else {
            result.push_back(list2[j].documentId);
            j++;
        }
    }

    while (i < list1.size()) {
        result.push_back(list1[i++].documentId);
    }
    while (j < list2.size()) {
        result.push_back(list2[j++].documentId);
    }
    return result;
}

std::vector<int> QueryParser::excludePostings(const std::vector<Posting>& list1, const std::vector<Posting>& list2) {
    std::vector<int> result;
    size_t i = 0, j = 0;

    while (i < list1.size() && j < list2.size()) {
        if (list1[i].documentId == list2[j].documentId) {
            i++;
            j++;
        } else if (list1[i].documentId < list2[j].documentId) {
            result.push_back(list1[i].documentId);
            i++;
        } else {
            j++;
        }
    }

    while (i < list1.size()) {
        result.push_back(list1[i++].documentId);
    }
    return result;
}