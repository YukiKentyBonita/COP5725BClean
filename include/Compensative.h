#ifndef COMPENSATIVE_H
#define COMPENSATIVE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <map>

class Compensative {
public:
    using AttrType = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;
    using Row = std::unordered_map<std::string, std::string>;
    using Data = std::vector<Row>;

    Compensative(const Data& data, const AttrType& attrs_type);

    void build();

    const auto& getOccurrenceList() const { return Occurrence_list; }
    const auto& getFrequencyList() const { return Frequency_list; }
    const auto& getOccurrence1() const { return Occurrence_1; }

private:
    Data data;
    AttrType attrs_type;

    std::unordered_map<std::string,
        std::unordered_map<std::string,
            std::unordered_map<std::string,
                std::unordered_map<std::string, double>>>> Occurrence_list;

    std::unordered_map<std::string,
        std::unordered_map<std::string, int>> Frequency_list;

    std::unordered_map<std::string,
        std::unordered_map<std::string,
            std::unordered_map<std::string,
                std::unordered_map<std::string, int>>>> Occurrence_1;

    void occur_and_fre();
    void correlate(int row_index, const std::string& attr_main);
    bool isValid(const std::string& attr, const std::string& value);
};

#endif // COMPENSATIVE_H
