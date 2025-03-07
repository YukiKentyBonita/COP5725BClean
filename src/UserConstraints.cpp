#include "../include/UserConstraints.h"
#include "../dataset.h"  // Include dataset.h to get DataFrame definition


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

// Build UC from a JSON file
void UC::build_from_json(const std::string& jpath) {
    try {
        std::ifstream file(jpath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open JSON file.");
        }

        std::string line;
        while (std::getline(file, line)) {
            // Look for "pattern" field in the JSON-like data
            if (line.find("pattern") != std::string::npos) {
                // Extract the part after ":"
                size_t pos = line.find(":");
                std::string pattern = line.substr(pos + 1);
                
                // Handle the case where pattern is "null" in the JSON
                if (pattern == "null") {
                    pattern = "";  // Set to empty string or handle as needed
                }
                
                // Now add the pattern to the map
                std::unordered_map<std::string, std::string> pattern_map;
                pattern_map["pattern"] = pattern;  // Assign the pattern (either empty or valid string)
                res["pattern"] = pattern_map;
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