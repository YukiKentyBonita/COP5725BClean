#ifndef USERCONSTRAINTS_H
#define USERCONSTRAINTS_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <regex>
#include <fstream>
#include <sstream>
#include <algorithm>

// Forward declare DataFrame (it is already defined in dataset.h)
struct DataFrame;

// Define the UC (User Constraints) class
class UC {
public:
    UC(const DataFrame& data);  // Now takes a reference to DataFrame

    // Get all constraints
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> get_uc();

    // Default setting to find min/max values for an attribute
    std::string default_setting(const std::string& attr, const std::string& type, const std::string& name);

    // Build the user constraints for a specific attribute
    void build(const std::string& attr, const std::string& type = "Categorical", const std::string& min_v = "", 
               const std::string& max_v = "", const std::string& null_allow = "N", const std::string& repairable = "Y", 
               const std::string& pattern = "");

    // Build UC from a JSON file
    void build_from_json(const std::string& jpath);

    // Print the user constraints for an attribute
    void edit(const std::string& df_attr, const std::string& uc_attr, const std::string& uc_v);

    // Discover patterns in each column using regex
    std::unordered_map<std::string, std::string> PatternDiscovery();

private:
    const DataFrame& data;  // Now holding a reference to DataFrame
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> res;

    // Helper function to extract values from a specific column
    std::vector<std::string> get_column_values(const std::string& attr);

    // Placeholder function to simulate pattern discovery
    std::string discover_pattern_in_column(const std::string& col);
};

#endif // USERCONSTRAINTS_H