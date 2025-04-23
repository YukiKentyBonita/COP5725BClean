// Test using 
// compensative->printFrequencyList(frequencyList);
// compensative->printOccurrence1(occurrence_1);
// compensative->printOccurrenceList(occurrenceList);

#ifndef COMPENSATIVE_H
#define COMPENSATIVE_H

#include <unordered_map>
#include <string>
#include <vector>
#include <map>
#include "dataset.h"  // DataFrame and AttrInfo

using std::string;
using std::vector;
using std::unordered_map;
using std::map;

using Row = unordered_map<string, string>;
using Data = vector<Row>;
// Update AttrType to match BayesianClean’s attribute type
using AttrType = map<string, AttrInfo>;

class Compensative {
public:
    Compensative(const DataFrame& dataFrame, const AttrType& attrs_type);

    void build();

    // Getters for BayesianClean to use
    const unordered_map<string,
        unordered_map<string,
            unordered_map<string, unordered_map<string, double>>>>& getOccurrenceList() const {
        return Occurrence_list;
    }

    const unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, int>>>>& getOccurrence1() const {
        return Occurrence_1;
    }

    const unordered_map<string,
        unordered_map<string, int>>& getFrequencyList() const {
        return Frequency_list;
    }

    // -------- For debugging -----------

    // Print frequencyList: unordered_map<string, unordered_map<string, int>>
    void printFrequencyList(const unordered_map<string, unordered_map<string, int>>& frequencyList);
    // Print occurrence_1: unordered_map<string, unordered_map<string, unordered_map<string, unordered_map<string, int>>>>
    void printOccurrence1(const unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, int>>>>& occurrence_1);
    // Print occurrenceList: unordered_map<string, unordered_map<string, unordered_map<string, unordered_map<string, double>>>>
    void printOccurrenceList(const unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, double>>>>& occurrenceList);

private:
    void occur_and_fre();
    void correlate(int row_index, const string& attr_main);
    bool isValid(const string& attr, const string& value);

    Data data;
    AttrType attrs_type;

    unordered_map<string,
        unordered_map<string,
            unordered_map<string, unordered_map<string, double>>>> Occurrence_list;

    unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, int>>>> Occurrence_1;

    unordered_map<string,
        unordered_map<string, int>> Frequency_list;
};

#endif // COMPENSATIVE_H