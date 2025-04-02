#ifndef DATASET_H
#define DATASET_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <cmath>
#include <algorithm>

// Use the standard namespace for brevity.
using namespace std;

/*
 * A simple structure to represent a DataFrame.
 */
struct DataFrame {
    vector<string> columns;
    vector<vector<string>> rows;
    
};

/*
 * Structure to hold attribute information.
 */
struct AttrInfo {
    string pattern; // if empty then no regex is applied
    string type;    // e.g., "Numerical"
    string allowNull;

    AttrInfo(const string& pat = "", const string& typ = "", const string& nullOK = "N")
        : pattern(pat), type(typ), allowNull(nullOK) {}
};

/*
 * The Dataset class encapsulates methods for loading and processing CSV data,
 * as well as comparing two datasets.
 */
class Dataset {
public:
    string tags;  // Default tag for missing cell values
    map<pair<int, string>, string> actual_error;  // Stores differences between two DataFrames
    mutex error_mutex;  // Mutex for thread safety

    // Constructor
    Dataset();

    // Reads a CSV file and returns a DataFrame.
    DataFrame get_data(const string& path);

    // Filters the DataFrame to include only columns specified in attr_type.
    DataFrame get_real_data(const DataFrame& data, const map<string, AttrInfo>& attr_type);
     // Function to print DataFrame (for debugging)
    void print_dataframe(const DataFrame& df) const;

    // Preprocesses the data by applying regex patterns and generating candidate rows.
    DataFrame pre_process_data(const DataFrame& data, const map<string, AttrInfo>& attr_type);

    // Compares two DataFrames cell by cell and records differences.
    void get_actual_error(const DataFrame& df1, const DataFrame& df2);

    // Wrapper that calls get_actual_error and returns the error map.
    map<pair<int, string>, string> get_error(const DataFrame& df1, const DataFrame& df2);

private:
    // Helper function: DFS that generates candidate row combinations.
    void dfs_line(vector<vector<string>>& re_lines,
                  vector<string>& current_line,
                  int attr_index,
                  const vector<string>& keys,
                  const vector<vector<string>>& values);

    // Generates candidate rows for a given row.
    vector<vector<string>> change_add_line(const vector<string>& data_line,
                                             int line_index,
                                             const vector<vector<string>>& add_lines,
                                             const vector<string>& attr_line,
                                             const vector<string>& attrs);
};

#endif // DATASET_H