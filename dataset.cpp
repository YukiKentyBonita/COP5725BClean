#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <map>
#include <thread>
#include <mutex>
#include <cmath>
#include <algorithm>

// Use the standard namespace for brevity.
using namespace std;

/*
 * A simple structure to represent a DataFrame.
 * - columns: vector of column names.
 * - rows: vector of rows, where each row is a vector of strings.
 */
struct DataFrame {
    vector<string> columns;
    vector<vector<string>> rows;
};

/*
 * Structure to hold attribute information. This is analogous to the attr_type
 * dictionary in the Python code. Each attribute has:
 * - pattern: a regex pattern (if empty, then no regex is applied).
 * - type: a string describing the type (e.g., "Numerical").
 */
struct AttrInfo {
    string pattern; // if empty then no pattern is applied
    string type;    // e.g., "Numerical"
};

/*
 * The Dataset class encapsulates methods for loading CSV data,
 * processing it (including applying regex, handling missing values,
 * and generating multiple candidate rows via a DFS combination),
 * and comparing two datasets to extract cell-level differences.
 */
class Dataset {
public:
    // A default tag to use for missing (null) cell values.
    string tags;
    // A map to store cells that differ between two datasets.
    // The key is a pair (row index, column name) and the value is the differing cell value from df2.
    map<pair<int, string>, string> actual_error;
    // Mutex to protect concurrent access to actual_error in multi-threaded code.
    mutex error_mutex;

    // Constructor: initializes the default tag.
    Dataset() : tags("A Null Cell") {}

    /*
     * Reads a CSV file from a given path.
     * The first line is assumed to be the header (column names).
     * Each subsequent line is split by commas into cells.
     */
    DataFrame get_data(const string& path) {
        DataFrame df;
        ifstream file(path);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << path << endl;
            return df;
        }
        string line;
        // Read header line and parse column names.
        if (getline(file, line)) {
            stringstream ss(line);
            string col;
            while (getline(ss, col, ',')) {
                // Optionally remove whitespace.
                col.erase(remove(col.begin(), col.end(), ' '), col.end());
                df.columns.push_back(col);
            }
        }
        // Read remaining lines as rows.
        while (getline(file, line)) {
            vector<string> row;
            stringstream ss(line);
            string cell;
            while (getline(ss, cell, ',')) {
                row.push_back(cell);
            }
            // Ensure each row has the same number of columns.
            while (row.size() < df.columns.size()) {
                row.push_back("");
            }
            df.rows.push_back(row);
        }
        file.close();
        return df;
    }

    /*
     * Filters the original DataFrame to only include the columns specified in attr_type.
     * For any missing value (an empty string), it replaces it with the default tag.
     */
    DataFrame get_real_data(const DataFrame& data, const map<string, AttrInfo>& attr_type) {
        DataFrame df;
        // Use the keys of attr_type as the new column order.
        for (const auto& kv : attr_type) {
            df.columns.push_back(kv.first);
        }
        // Build a mapping from column name to its index in the original DataFrame.
        map<string, int> colIndex;
        for (int i = 0; i < data.columns.size(); i++) {
            colIndex[data.columns[i]] = i;
        }
        // For each row in the original data, pick the selected columns.
        for (const auto& row : data.rows) {
            vector<string> newRow;
            for (const auto& col : df.columns) {
                string value = "";
                if (colIndex.find(col) != colIndex.end()) {
                    value = row[colIndex[col]];
                }
                if (value.empty()) {
                    value = tags;
                }
                newRow.push_back(value);
            }
            df.rows.push_back(newRow);
        }
        return df;
    }

    /*
     * A recursive helper function to generate all combinations of candidate values
     * for the specified attributes.
     *
     * Parameters:
     * - re_lines: A reference to a vector where each complete combination (row) is added.
     * - current_line: The current row being modified.
     * - attr_index: The current attribute index (used for recursion).
     * - keys: The list of attribute names (order assumed to match the DataFrame columns).
     * - values: For each attribute, a vector of candidate values.
     */
    void dfs_line(vector<vector<string>>& re_lines,
                  vector<string>& current_line,
                  int attr_index,
                  const vector<string>& keys,
                  const vector<vector<string>>& values) {
        if (attr_index == keys.size()) {
            // Base case: all attributes have been updated; add the combination.
            re_lines.push_back(current_line);
            return;
        }
        // Iterate over all candidate values for the current attribute.
        for (const auto& val : values[attr_index]) {
            // Assume the order of keys corresponds to the column order.
            int index = attr_index; 
            string old_value = current_line[index];
            current_line[index] = val;
            // Recurse to process the next attribute.
            dfs_line(re_lines, current_line, attr_index + 1, keys, values);
            // Restore the original value (backtracking).
            current_line[index] = old_value;
        }
    }

    /*
     * Given a single row (data_line) and the candidate values for certain attributes
     * (add_lines), this function generates all new rows that result from replacing the original
     * cell values with each candidate value. The attr_line vector holds the attribute names
     * corresponding to add_lines, and attrs is the full list of attributes.
     *
     * Returns a vector of new rows (each row is a vector of strings).
     */
    vector<vector<string>> change_add_line(const vector<string>& data_line,
                                             int line_index,
                                             const vector<vector<string>>& add_lines,
                                             const vector<string>& attr_line,
                                             const vector<string>& attrs) {
        // Copy the original row so it can be modified.
        vector<string> current_line = data_line;
        vector<vector<string>> result_lines;
        // Use DFS to generate all candidate combinations.
        dfs_line(result_lines, current_line, 0, attr_line, add_lines);
        return result_lines;
    }

    /*
     * Preprocesses the original data for training and cleaning.
     *
     * For each row:
     * 1. Selects the columns defined in attr_type.
     * 2. For each attribute, if a regex pattern is provided, applies it to the cell value.
     *    If a match is found and the attribute type is "Numerical", it converts the string to a number,
     *    and if the number is an integer, it formats it as such.
     * 3. Generates new rows (if needed) based on the candidate values using DFS.
     *
     * Returns a new DataFrame containing the preprocessed rows.
     */
    DataFrame pre_process_data(const DataFrame& data, const map<string, AttrInfo>& attr_type) {
        // Extract the list of attributes to process.
        vector<string> attrs;
        for (const auto& kv : attr_type) {
            attrs.push_back(kv.first);
        }
        // Create the training DataFrame with these columns.
        DataFrame df_train;
        df_train.columns = attrs;

        // For convenience, build a mapping from original column names to indices.
        map<string, int> colIndex;
        for (int j = 0; j < data.columns.size(); j++) {
            colIndex[data.columns[j]] = j;
        }

        // Process each row.
        for (int i = 0; i < data.rows.size(); i++) {
            const auto& row = data.rows[i];
            // Will store candidate values for each attribute.
            vector<vector<string>> add_lines;
            // Also record the attribute names (order matters for DFS).
            vector<string> attr_line;
            // Process each attribute.
            for (const auto& at : attrs) {
                attr_line.push_back(at);
                vector<string> candidates;
                // Retrieve the cell value; if not present, use the default tag.
                string cell_value = tags;
                if (colIndex.find(at) != colIndex.end()) {
                    cell_value = row[colIndex[at]];
                }
                // If a regex pattern is specified, use it to search within the cell.
                if (!attr_type.at(at).pattern.empty()) {
                    regex pattern(attr_type.at(at).pattern);
                    smatch match;
                    if (regex_search(cell_value, match, pattern)) {
                        string val = match.str(0);
                        // If the attribute is numerical, convert and reformat if necessary.
                        if (attr_type.at(at).type == "Numerical") {
                            try {
                                double num = stod(val);
                                // If the number is an integer, convert accordingly.
                                if (floor(num) == num) {
                                    val = to_string(static_cast<int>(num));
                                } else {
                                    val = to_string(num);
                                }
                            } catch (...) {
                                // If conversion fails, retain the original value.
                            }
                        }
                        candidates.push_back(val);
                    } else {
                        candidates.push_back(cell_value);
                    }
                } else {
                    // If no pattern is given, simply use the cell's value.
                    candidates.push_back(cell_value);
                }
                add_lines.push_back(candidates);
            }
            // Generate all combinations (new rows) from the candidate values.
            vector<vector<string>> new_rows = change_add_line(row, i, add_lines, attr_line, attrs);
            // Append the generated rows to df_train.
            for (auto& new_row : new_rows) {
                df_train.rows.push_back(new_row);
            }
        }
        return df_train;
    }

    /*
     * Compares two DataFrames (df1 and df2) cell by cell.
     * For any cell that differs between df1 and df2, the function stores the value from df2
     * into the actual_error map, using the key (row index, column name).
     *
     * To speed up the comparison, the rows are split among multiple threads.
     */
    void get_actual_error(const DataFrame& df1, const DataFrame& df2) {
        // Assume both DataFrames have the same dimensions and column order.
        int num_rows = df1.rows.size();
        int num_cols = df1.columns.size();
        // Determine how many threads to use.
        unsigned int num_threads = thread::hardware_concurrency();
        if (num_threads == 0) {
            num_threads = 2; // fallback default
        }
        vector<thread> threads;
        // Divide the rows into chunks for each thread.
        int chunk_size = (num_rows + num_threads - 1) / num_threads;

        // The worker lambda compares a range of rows.
        auto worker = [this, &df1, &df2, num_cols](int start, int end) {
            for (int i = start; i < end; i++) {
                for (int j = 0; j < num_cols; j++) {
                    if (df1.rows[i][j] != df2.rows[i][j]) {
                        // Lock the mutex before modifying the shared map.
                        lock_guard<mutex> lock(this->error_mutex);
                        this->actual_error[{i, df1.columns[j]}] = df2.rows[i][j];
                    }
                }
            }
        };

        // Launch threads for each chunk.
        for (unsigned int t = 0; t < num_threads; t++) {
            int start = t * chunk_size;
            int end = min(start + chunk_size, num_rows);
            threads.emplace_back(worker, start, end);
        }
        // Wait for all threads to finish.
        for (auto& th : threads) {
            if (th.joinable())
                th.join();
        }
        cout << "++++++++++++ " << actual_error.size() << " error cells collected ++++++++++++" << endl;
    }

    /*
     * A wrapper that first calls get_actual_error (which populates the actual_error map)
     * and then returns it.
     */
    map<pair<int, string>, string> get_error(const DataFrame& df1, const DataFrame& df2) {
        get_actual_error(df1, df2);
        return actual_error;
    }
};
