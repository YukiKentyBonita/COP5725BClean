#include "../include/UserConstraints.h"
#include "../dataset.h"  // Include dataset.h to get DataFrame definition
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <vector>
#include <utility>



// // DataFrame method to load data from a CSV file
// bool DataFrame::load_from_csv(const std::string& path) {
//     std::ifstream file(path);
//     if (!file.is_open()) {
//         std::cerr << "Failed to open file: " << path << std::endl;
//         return false;
//     }

//     std::string line;
//     // Read the columns from the first line
//     if (std::getline(file, line)) {
//         std::stringstream ss(line);
//         std::string col;
//         while (std::getline(ss, col, ',')) {
//             columns.push_back(col);
//         }
//     }

//     // Read the data rows
//     while (std::getline(file, line)) {
//         std::vector<std::string> row;
//         std::stringstream ss(line);
//         std::string cell;
//         while (std::getline(ss, cell, ',')) {
//             row.push_back(cell);
//         }
//         data.push_back(row);
//     }

//     file.close();
//     return true;
// }

// UC class constructor now takes a reference to the DataFrame
UC::UC(const DataFrame& data) : data(data) {}

// Get all constraints
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> UC::get_uc() {
    return res;
}

// Default setting to find min/max values for an attribute
std::string UC::default_setting(const std::string& attr, const std::string& type, const std::string& name) {
    std::vector<std::string> domain = get_column_values(attr);
    std::string result;

    if (type == "Categorical") {
        if (name == "min_v") {
            result = *std::min_element(domain.begin(), domain.end(), 
                [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
        }
        else if (name == "max_v") {
            result = *std::max_element(domain.begin(), domain.end(), 
                [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
        }
    } 
    else if (type == "Numerical") {
        result = *std::min_element(domain.begin(), domain.end());
        if (name == "max_v") {
            result = *std::max_element(domain.begin(), domain.end());
        }
    }

    return result;
}

// Build the user constraints for a specific attribute
void UC::build(const std::string& attr, const std::string& type, const std::string& min_v, 
               const std::string& max_v, const std::string& null_allow, const std::string& repairable, 
               const std::string& pattern) {
    if (std::find(data.columns.begin(), data.columns.end(), attr) == data.columns.end()) {
        std::cout << "No such attribute: " << attr << std::endl;
        return;
    }

    std::string min_value = min_v.empty() ? default_setting(attr, type, "min_v") : min_v;
    std::string max_value = max_v.empty() ? default_setting(attr, type, "max_v") : max_v;

    res[attr] = {{"type", type}, {"min_length", min_value}, {"max_length", max_value}, 
                 {"AllowNull", null_allow}, {"repairable", repairable}, {"pattern", pattern}};
    std::cout << "User constraint for attribute '" << attr << "' has been set." << std::endl;
}


// Helper function to trim whitespace from both ends of a string
std::string trim(const std::string &s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

void UC::build_from_json(const std::string& jpath) {
    try {
        std::ifstream file(jpath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open JSON file.");
        }
        std::string line;
        std::string currentAttr = "";
        std::unordered_map<std::string, std::string> constraintMap;
        bool insideBlock = false;

        while (std::getline(file, line)) {
            line = trim(line);
            // Skip empty lines or just braces
            if (line.empty() || line == "{" || line == "}" || line == "},")
                continue;

            // Check if line defines an attribute block e.g., "ounces": {
            size_t pos = line.find("\":");
            if (pos != std::string::npos && line.find("{") != std::string::npos) {
                // Finish previous attribute block if any
                if (insideBlock && !currentAttr.empty()) {
                    res[currentAttr] = constraintMap;
                    constraintMap.clear();
                }
                // Extract attribute name, removing leading quote
                currentAttr = line.substr(1, pos - 1);
                insideBlock = true;
                continue;
            }

            // If inside a block, parse key-value pairs until a closing brace
            if (insideBlock) {
                // If we reach the end of the block for an attribute
                if (line[0] == '}') {
                    // Save current attribute's constraints
                    if (!currentAttr.empty()) {
                        res[currentAttr] = constraintMap;
                    }
                    constraintMap.clear();
                    currentAttr = "";
                    insideBlock = false;
                    continue;
                }

                // Expecting a line like "type": "Numerical", or "min_length": 0,
                // Remove comma at the end if present
                if (line.back() == ',')
                    line.pop_back();
                // Split at colon
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = trim(line.substr(0, colonPos));
                    std::string value = trim(line.substr(colonPos + 1));
                    // Remove quotes from key if present
                    if (!key.empty() && key.front() == '"') {
                        key = key.substr(1, key.size() - 2);
                    }
                    // Process value: remove quotes if string, or handle null
                    if (!value.empty() && value.front() == '"') {
                        // Remove leading and trailing quotes
                        value = value.substr(1, value.size() - 2);
                    } else if (value == "null") {
                        value = "";  // convert JSON null to empty string
                    }
                    // Store the key-value pair in constraintMap
                    constraintMap[key] = value;
                }
            }
        }
        // If the last attribute block wasn't closed with a '}', add it now.
        if (insideBlock && !currentAttr.empty()) {
            res[currentAttr] = constraintMap;
        }

        file.close();
    } catch (const std::exception& e) {
        std::cout << "Error reading JSON: " << e.what() << std::endl;
    }
}
// Print the user constraints for an attribute
void UC::edit(const std::string& df_attr, const std::string& uc_attr, const std::string& uc_v) {
    if (res.find(df_attr) == res.end()) {
        std::cout << "No such attribute: " << df_attr << std::endl;
        return;
    }

    if (res[df_attr].find(uc_attr) == res[df_attr].end()) {
        std::cout << "No such UC attribute: " << uc_attr << std::endl;
        return;
    }

    res[df_attr][uc_attr] = uc_v;
    std::cout << "Attribute modified" << std::endl;
}

// Discover patterns in each column using regex
std::unordered_map<std::string, std::string> UC::PatternDiscovery() {
    std::unordered_map<std::string, std::string> patterns;
    for (const auto& col : data.columns) {
        std::string pattern = discover_pattern_in_column(col);
        patterns[col] = pattern;
    }
    return patterns;
}

// Helper function to extract values from a specific column
std::vector<std::string> UC::get_column_values(const std::string& attr) {
    std::vector<std::string> values;
    for (const auto& row : data.rows) {
        values.push_back(row[std::distance(data.columns.begin(), 
            std::find(data.columns.begin(), data.columns.end(), attr))]);
    }
    return values;
}

// Placeholder function to simulate pattern discovery

// Improved helper function to extract a more advanced pattern from a column's data
std::string UC::discover_pattern_in_column(const std::string& col) {
    std::vector<std::string> values = get_column_values(col);
    if (values.empty()) return "";

    // Preprocess: trim whitespace for each value (optional)
    for (auto &val : values) {
        // A simple trim (you could use a more robust version)
        size_t start = val.find_first_not_of(" \t");
        size_t end = val.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos)
            val = val.substr(start, end - start + 1);
    }

    // Define candidate regex patterns.
    std::vector<std::pair<std::string, std::regex>> candidates = {
        {"\\d+(\\.\\d+)?", std::regex("^\\d+(\\.\\d+)?$")},                  // Numeric (integer or float)
        {"[A-Za-z]+", std::regex("^[A-Za-z]+$")},                             // Alphabetical only
        {"[A-Za-z0-9]+", std::regex("^[A-Za-z0-9]+$")},                       // Alphanumeric
        {"\\d{4}-\\d{2}-\\d{2}", std::regex("^\\d{4}-\\d{2}-\\d{2}$")},       // Date in YYYY-MM-DD
        {"\\w+@\\w+\\.\\w+", std::regex("^\\w+@\\w+\\.\\w+$")},               // Email (simple)
        {"[A-Za-z\\.\\s]+", std::regex("^[A-Za-z\\.\\s]+$")}                  // Allow letters, dots, and spaces
    };

    // First, check for a perfect match.
    for (const auto &candidate : candidates) {
        bool allMatch = true;
        for (const auto &val : values) {
            if (val.empty()) continue;
            if (!std::regex_match(val, candidate.second)) {
                allMatch = false;
                break;
            }
        }
        if (allMatch) {
            return candidate.first;
        }
    }

    // Compute match rate for each candidate.
    int bestCandidateIndex = -1;
    double bestMatchRate = 0.0;
    for (size_t i = 0; i < candidates.size(); i++) {
        int matchCount = 0;
        int total = 0;
        for (const auto &val : values) {
            if (val.empty()) continue;
            total++;
            if (std::regex_match(val, candidates[i].second)) {
                matchCount++;
            }
        }
        double rate = (total > 0) ? static_cast<double>(matchCount) / total : 0.0;
        if (rate > bestMatchRate) {
            bestMatchRate = rate;
            bestCandidateIndex = static_cast<int>(i);
        }
    }

    // Lowered threshold to 0.7 for example purposes
    if (bestMatchRate >= 0.8 && bestCandidateIndex != -1) {
        return candidates[bestCandidateIndex].first;
    }

    // Fallback generic pattern.
    return ".+";
}