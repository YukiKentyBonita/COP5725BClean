#include "../include/Compensative.h"
#include <iostream>
#include <regex>
#include <cmath>

Compensative::Compensative(const Data& data, const AttrType& attrs_type)
    : data(data), attrs_type(attrs_type) {}

void Compensative::build() {
    occur_and_fre();
}

void Compensative::occur_and_fre() {
    Frequency_list.clear();
    Occurrence_list.clear();
    Occurrence_1.clear();

    // Frequency counting
    for (const auto& row : data) {
        for (const auto& [attr, val] : row) {
            Frequency_list[attr][val]++;
        }
    }

    // Compute co-occurrence
    for (size_t i = 0; i < data.size(); ++i) {
        for (const auto& [attr_main, _] : data[i]) {
            correlate(i, attr_main);
        }
    }
}

bool Compensative::isValid(const std::string& attr, const std::string& value) {
    auto& info = attrs_type.at(attr);
    bool allowNull = (info.at("AllowNull") == "Y");
    std::string pattern = info.at("pattern");

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
