#include "../include/UserConstraints.h"
#include "../dataset.h"  // Include dataset.h to get DataFrame definition
#include <sstream>
#include <algorithm>
#include <cctype>


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
        std::string pattern = discover_pattern_in_column(col);  // Placeholder function for pattern discovery
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
std::string UC::discover_pattern_in_column(const std::string& col) {
    return "Regex for " + col;  // Return a dummy regex pattern
}