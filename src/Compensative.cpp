// #include "../include/Compensative.h"
// #include <iostream>
// #include <regex>
// #include <cmath>

// Compensative::Compensative(const Data& data, const AttrType& attrs_type)
//     : data(data), attrs_type(attrs_type) {}

// void Compensative::build() {
//     occur_and_fre();
// }

// void Compensative::occur_and_fre() {
//     Frequency_list.clear();
//     Occurrence_list.clear();
//     Occurrence_1.clear();

//     // Frequency counting
//     for (const auto& row : data) {
//         for (const auto& [attr, val] : row) {
//             Frequency_list[attr][val]++;
//         }
//     }

//     // Compute co-occurrence
//     for (size_t i = 0; i < data.size(); ++i) {
//         for (const auto& [attr_main, _] : data[i]) {
//             correlate(i, attr_main);
//         }
//     }
// }

// bool Compensative::isValid(const std::string& attr, const std::string& value) {
//     auto& info = attrs_type.at(attr);
//     bool allowNull = (info.at("AllowNull") == "Y");
//     std::string pattern = info.at("pattern");

//     if (!allowNull && value == "A Null Cell")
//         return false;

//     if (!pattern.empty()) {
//         try {
//             std::regex regex_pattern(pattern);
//             return std::regex_match(value, regex_pattern);
//         } catch (...) {
//             return false;
//         }
//     }

//     return true;
// }

// void Compensative::correlate(int row_index, const std::string& attr_main) {
//     int weight = attrs_type.size() * attrs_type.size();
//     double pen_weight = weight;
//     double confident = 1.0;

//     const std::string& main_val = data[row_index].at(attr_main);

//     if (!isValid(attr_main, main_val)) {
//         pen_weight -= 2.0 * weight * weight;
//         confident = 0;
//     }

//     auto& occ_main = Occurrence_list[attr_main][main_val];
//     auto& occ1_main = Occurrence_1[attr_main][main_val];

//     for (const auto& [attr_vice, _] : data[row_index]) {
//         if (attr_main == attr_vice) continue;

//         const std::string& vice_val = data[row_index].at(attr_vice);
//         if (!isValid(attr_vice, vice_val)) {
//             confident *= 0.5;
//             pen_weight -= 2.0 * weight;
//         }

//         occ1_main[attr_vice][vice_val] += 1;

//         double& score = occ_main[attr_vice][vice_val];
//         if (confident >= 0.5) {
//             score += weight;
//         } else if (confident == 0) {
//             score = 0;
//         } else {
//             score += pen_weight;
//         }
//         score = std::max(0.0, score);
//     }
// }

#include "Compensative.h"
#include <iostream>
#include <regex>
#include <cmath>
#include <algorithm>

static inline std::string canonical(std::string s) {
    std::string out;
    for(char ch:s)
        if(!std::isspace(static_cast<unsigned char>(ch)) && ch!='%')
            out.push_back(std::tolower(static_cast<unsigned char>(ch)));
    return out;
}

Compensative::Compensative(const DataFrame& dataFrame, const AttrType& attrs_type)
    : attrs_type(attrs_type)
{
    // Convert the DataFrame to our internal Data (vector<Row>).
    // Each row in dataFrame.rows is a vector<string>; we pair each value with its column name.
    for (const auto &rowVec : dataFrame.rows) {
        Row row;
        for (size_t i = 0; i < dataFrame.columns.size() && i < rowVec.size(); ++i) {
            row[dataFrame.columns[i]] = rowVec[i];
        }
        data.push_back(row);
    }
}

void Compensative::build() {
    occur_and_fre();
}

void Compensative::occur_and_fre() {
    Frequency_list.clear();
    Occurrence_list.clear();
    Occurrence_1.clear();

    // Frequency counting: count occurrences of each attribute value.
    for (const auto& row : data) {
        for (const auto& [attr, val] : row) {
            Frequency_list[attr][val]++;
        }
    }

    // Compute co-occurrence for each row and attribute.
    for (size_t i = 0; i < data.size(); ++i) {
        for (const auto& [attr_main, _] : data[i]) {
            correlate(i, attr_main);
        }
    }
}

bool Compensative::isValid(const std::string& attr, const std::string& value) {
    // Get the attribute info (pattern, allowNull, etc.) from the AttrType.
    const AttrInfo& info = attrs_type.at(attr);
    bool allowNull = (info.allowNull == "Y");
    std::string pattern = info.pattern;

    if (!allowNull && value == "A Null Cell")
        return false;

    if (!pattern.empty()) {
        try {
            std::regex regex_pattern(pattern);
            return std::regex_match(value, regex_pattern);
        } catch (...) {
            return false;
        }
    }

    return true;
}

void Compensative::correlate(int row_index, const std::string& attr_main) {
    int weight = attrs_type.size() * attrs_type.size();
    double pen_weight = weight;
    double confident = 1.0;

    const std::string& main_val = data[row_index].at(attr_main);

    if (!isValid(attr_main, main_val)) {
        pen_weight -= 2.0 * weight * weight;
        confident = 0;
    }

    auto& occ_main = Occurrence_list[attr_main][main_val];
    auto& occ1_main = Occurrence_1[attr_main][main_val];

    for (const auto& [attr_vice, _] : data[row_index]) {
        if (attr_main == attr_vice) continue;

        const std::string& vice_val = data[row_index].at(attr_vice);
        if (!isValid(attr_vice, vice_val)) {
            confident *= 0.5;
            pen_weight -= 2.0 * weight;
        }

        occ1_main[attr_vice][vice_val] += 1;

        double& score = occ_main[attr_vice][vice_val];
        if (confident >= 0.5) {
            score += weight;
        } else if (confident == 0) {
            score = 0;
        } else {
            score += pen_weight;
        }
        score = std::max(0.0, score);
    }
}

// Print frequencyList: unordered_map<string, unordered_map<string, int>>
void Compensative::printFrequencyList(const unordered_map<string, unordered_map<string, int>>& frequencyList) {
    std::cout << "=== Frequency List ===\n";
    for (const auto& [attr, val_map] : frequencyList) {
        std::cout << "Attribute: " << attr << "\n";
        for (const auto& [val, freq] : val_map) {
            std::cout << "  Value: " << val << " -> Freq: " << freq << "\n";
        }
    }
    std::cout << std::endl;
}

// Print occurrence_1: unordered_map<string, unordered_map<string, unordered_map<string, unordered_map<string, int>>>>
void Compensative::printOccurrence1(const unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, int>>>>& occurrence_1) {
    std::cout << "=== Occurrence 1 ===\n";
    for (const auto& [attr_main, v_map] : occurrence_1) {
        std::cout << "Main Attribute: " << attr_main << "\n";
        for (const auto& [val_main, attr_corr_map] : v_map) {
            std::cout << "  Main Value: " << val_main << "\n";
            for (const auto& [attr_corr, val_corr_map] : attr_corr_map) {
                std::cout << "    Correlated Attr: " << attr_corr << "\n";
                for (const auto& [val_corr, count] : val_corr_map) {
                    std::cout << "      Val: " << val_corr << " -> Count: " << count << "\n";
                }
            }
        }
    }
    std::cout << std::endl;
}

// Print occurrenceList: unordered_map<string, unordered_map<string, unordered_map<string, unordered_map<string, double>>>>
void Compensative::printOccurrenceList(const unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, double>>>>& occurrenceList) {
    std::cout << "=== Occurrence List ===\n";
    for (const auto& [attr_main, v_map] : occurrenceList) {
        std::cout << "Main Attribute: " << attr_main << "\n";
        for (const auto& [val_main, attr_corr_map] : v_map) {
            std::cout << "  Main Value: " << val_main << "\n";
            for (const auto& [attr_corr, val_corr_map] : attr_corr_map) {
                std::cout << "    Correlated Attr: " << attr_corr << "\n";
                for (const auto& [val_corr, prob] : val_corr_map) {
                    std::cout << "      Val: " << val_corr << " -> Prob: " << prob << "\n";
                }
            }
        }
    }
    std::cout << std::endl;
}