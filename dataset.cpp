#include "dataset.h"

Dataset::Dataset() : tags("A Null Cell") { }

DataFrame Dataset::get_data(const string& path) {
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
        while (row.size() < df.columns.size()) {
            row.push_back("");
        }
        df.rows.push_back(row);
    }
    file.close();
    return df;
}

// Function to filter columns and replace missing values
DataFrame Dataset::get_real_data(const DataFrame& data, const map<string, AttrInfo>& attr_type) {
    DataFrame df;

    // Extract only relevant column names
    for (const auto& kv : attr_type) {
        df.columns.push_back(kv.first);
    }

    // Map column names to their index in the original DataFrame
    unordered_map<string, int> colIndex;
    for (size_t i = 0; i < data.columns.size(); i++) {
        colIndex[data.columns[i]] = i;
    }

    // Copy rows, preserving only selected columns
    for (const auto& row : data.rows) {
        vector<string> newRow;
        for (const auto& col : df.columns) {
            string value = "";
            if (colIndex.find(col) != colIndex.end()) {
                value = row[colIndex[col]];
            }
            if (value.empty()) {
                value = tags;  // Fill missing values with `tags`
            }
            newRow.push_back(value);
        }
        df.rows.push_back(newRow);
    }
    return df;
}

// Function to print DataFrame (for debugging)
void Dataset::print_dataframe(const DataFrame& df) const {
    cout << "Filtered DataFrame:\n";
    for (const auto& col : df.columns) {
        cout << col << "\t";
    }
    cout << "\n";

    for (const auto& row : df.rows) {
        for (const auto& cell : row) {
            cout << cell << "\t";
        }
        cout << "\n";
    }
}

void Dataset::dfs_line(vector<vector<string>>& re_lines,
                       vector<string>& current_line,
                       int attr_index,
                       const vector<string>& keys,
                       const vector<vector<string>>& values) {
    if (attr_index == keys.size()) {
        re_lines.push_back(current_line);
        return;
    }
    for (const auto& val : values[attr_index]) {
        int index = attr_index; 
        string old_value = current_line[index];
        current_line[index] = val;
        dfs_line(re_lines, current_line, attr_index + 1, keys, values);
        current_line[index] = old_value;
    }
}

vector<vector<string>> Dataset::change_add_line(const vector<string>& data_line,
                                                  int line_index,
                                                  const vector<vector<string>>& add_lines,
                                                  const vector<string>& attr_line,
                                                  const vector<string>& attrs) {
    vector<string> current_line = data_line;
    vector<vector<string>> result_lines;
    dfs_line(result_lines, current_line, 0, attr_line, add_lines);
    return result_lines;
}

DataFrame Dataset::pre_process_data(const DataFrame& data, const map<string, AttrInfo>& attr_type) {
    vector<string> attrs;
    for (const auto& kv : attr_type) {
        attrs.push_back(kv.first);
    }
    DataFrame df_train;
    df_train.columns = attrs;
    map<string, int> colIndex;
    for (int j = 0; j < data.columns.size(); j++) {
        colIndex[data.columns[j]] = j;
    }
    for (int i = 0; i < data.rows.size(); i++) {
        const auto& row = data.rows[i];
        vector<vector<string>> add_lines;
        vector<string> attr_line;
        for (const auto& at : attrs) {
            attr_line.push_back(at);
            vector<string> candidates;
            string cell_value = tags;
            if (colIndex.find(at) != colIndex.end()) {
                cell_value = row[colIndex[at]];
            }
            if (!attr_type.at(at).pattern.empty()) {
                regex pattern(attr_type.at(at).pattern);
                smatch match;
                if (regex_search(cell_value, match, pattern)) {
                    string val = match.str(0);
                    if (attr_type.at(at).type == "Numerical") {
                        try {
                            double num = stod(val);
                            if (floor(num) == num) {
                                val = to_string(static_cast<int>(num));
                            } else {
                                val = to_string(num);
                            }
                        } catch (...) {
                        }
                    }
                    candidates.push_back(val);
                } else {
                    candidates.push_back(cell_value);
                }
            } else {
                candidates.push_back(cell_value);
            }
            add_lines.push_back(candidates);
        }
        vector<vector<string>> new_rows = change_add_line(row, i, add_lines, attr_line, attrs);
        for (auto& new_row : new_rows) {
            df_train.rows.push_back(new_row);
        }
    }
    return df_train;
}

void Dataset::get_actual_error(const DataFrame& df1, const DataFrame& df2) {
    int num_rows = df1.rows.size();
    int num_cols = df1.columns.size();
    unsigned int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) {
        num_threads = 2;
    }
    vector<thread> threads;
    int chunk_size = (num_rows + num_threads - 1) / num_threads;

    auto worker = [this, &df1, &df2, num_cols](int start, int end) {
        for (int i = start; i < end; i++) {
            for (int j = 0; j < num_cols; j++) {
                if (df1.rows[i][j] != df2.rows[i][j]) {
                    lock_guard<mutex> lock(this->error_mutex);
                    this->actual_error[{i, df1.columns[j]}] = df2.rows[i][j];
                }
            }
        }
    };

    for (unsigned int t = 0; t < num_threads; t++) {
        int start = t * chunk_size;
        int end = min(start + chunk_size, num_rows);
        threads.emplace_back(worker, start, end);
    }
    for (auto& th : threads) {
        if (th.joinable())
            th.join();
    }
    cout << "++++++++++++ " << actual_error.size() << " error cells collected ++++++++++++" << endl;
}

map<pair<int, string>, string> Dataset::get_error(const DataFrame& df1, const DataFrame& df2) {
    get_actual_error(df1, df2);
    return actual_error;
}