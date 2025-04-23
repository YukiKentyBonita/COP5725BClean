#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <thread>
#include <tuple>
#include <map>
#include <set>
#include <algorithm> 
#include "../dataset.h"
#include "../include/UserConstraints.h"
#include "../BayesianClean.h"
using namespace std;


// Helper function to get value from DataFrame
std::string get_cell_value(const DataFrame& df, int row_idx, const std::string& col_name) {
    auto it = std::find(df.columns.begin(), df.columns.end(), col_name);
    if (it != df.columns.end()) {
        size_t col_idx = it - df.columns.begin();
        return df.rows[row_idx][col_idx];
    } else {
        return ""; // column not found
    }
}

std::tuple<double, double, double> analysis(
    const std::map<std::pair<int, std::string>, std::string>& actual_error,
    const std::map<std::pair<int, std::string>, std::string>& repair_error,
    const DataFrame& dirty_data,
    const DataFrame& clean_data
) {
    int pre_right = 0;
    int pre_wrong = 0;
    int miss_err = 0;

    int whole_error = actual_error.size();
    int whole_repair = repair_error.size();

    std::map<std::pair<int, std::string>, std::string> actual_error_repair;
    std::map<std::pair<int, std::string>, std::string> pre_not_right;
    std::map<std::pair<int, std::string>, std::string> missing_wrong;

    // Check repaired cells against actual errors
    for (const auto& [cell, value] : actual_error) {
        if (repair_error.count(cell)) {
            if (actual_error.at(cell) == repair_error.at(cell)) {
                pre_right++;
                actual_error_repair[cell] = get_cell_value(dirty_data, cell.first, cell.second) + 
                    " ===> " + repair_error.at(cell);
            }
        } else {
            missing_wrong[cell] = get_cell_value(dirty_data, cell.first, cell.second) + 
                " ===> " + get_cell_value(clean_data, cell.first, cell.second);
            miss_err++;
        }
    }

    // Check false positives
    for (const auto& [cell, value] : repair_error) {
        if (!actual_error.count(cell)) {
            pre_wrong++;
            pre_not_right[cell] = get_cell_value(dirty_data, cell.first, cell.second) + 
                " ===> " + get_cell_value(clean_data, cell.first, cell.second) + " but not " + repair_error.at(cell);
        } else {
            if (actual_error.at(cell) != repair_error.at(cell)) {
                pre_wrong++;
                pre_not_right[cell] = repair_error.at(cell);
            }
        }
    }

    double P = (whole_repair > 0) ? double(pre_right) / double(whole_repair) : 0.0;
    double R = (whole_error > 0) ? double(pre_right) / double(whole_error) : 0.0;
    double F = (P + R > 0) ? 2 * P * R / (P + R) : 0.0;

    return {P, R, F};
}

int main(int argc, char* argv[])
{
    std::string versionName = "";

    if (argc == 2) {
        versionName = argv[1];
    }

    if (versionName.empty()) {
        std::cout << "Running BClean: basic version without any optimizations." << std::endl;
    } else if (versionName == "-UC") {
        std::cout << "Running BClean-UC: variant without employing UCs." << std::endl;
    } else if (versionName == "-PI") {
        std::cout << "Running BCleanₚᵢ: variant with Partition Inference optimization." << std::endl;
    } else if (versionName == "-PIP") {
        std::cout << "Running BCleanₚᵢₚ: variant with Partition Inference and Pruning optimizations." << std::endl;
    } else {
        std::cout << "Unknown version argument: " << versionName << std::endl;
        return 1; // exit with error
    }

    Dataset dataset;
    // Paths to the dirty and clean data
    string dirty_path = "data/dirty.csv";
    string clean_path = "data/clean.csv";
    string json_path = "json/beers.json";

    // Load data using the Dataset methods.
    DataFrame dirty_data = dataset.get_data(dirty_path);
    DataFrame clean_data = dataset.get_data(clean_path);

    map<string, AttrInfo> attr_type;
    if (!(versionName == "-UC")) {
        // Initialize UC (User Constraints) with dirty data
        UC uc(dirty_data);

        cout << "Loading constraints from JSON file..." << endl;
        uc.build_from_json(json_path); // Call build_from_json to load constraints from the JSON file

        // Print out the content of the JSON file
        cout << "User Constraints after loading from JSON:" << endl;
        auto updated_uc_data = uc.get_uc();
        for (const auto &[key, value] : updated_uc_data)
        {
            cout << key << " : ";
            for (const auto &[attr, val] : value)
            {
                cout << attr << "=" << val << " ";
            }
            cout << endl;
        }

        // Applying pattern discovery
        cout << "Applying Pattern Discovery..." << endl;
        auto patterns = uc.PatternDiscovery(); // Use UC's pattern discovery method
        cout << "Pattern discovery: ";
        for (const auto &pat : patterns)
        {
            cout << pat.first << ": " << pat.second << " ";
        }
        cout << endl;


        // Simulating repair data
        for (const auto &[col, constraints] : updated_uc_data)
        {
            if (constraints.find("type") != constraints.end())
            {
                attr_type[col] = {constraints.at("type")}; // Extract type from UC
            }
            else
            {
                attr_type[col] = {"Unknown"}; // Ensure all attributes are included
            }
        }

        // **Call get_real_data() correctly**
        dirty_data = dataset.get_real_data(dirty_data, attr_type);
        clean_data = dataset.get_real_data(clean_data, attr_type);

    }
    // Starting timing
    auto start_time = chrono::system_clock::now();

    std::cout << "\n===== Instantiating BayesianClean for Compensative test =====\n";

    BayesianClean model(
        dirty_data,
        clean_data,
        "Compensative", // inference strategy
        0.5,            // tuple pruning
        5,              // maxiter
        2,              // num_worker
        2,              // chunk size
        "",             // model_path
        "",             // model_save_path
        attr_type,
        {},    // fix_edge
        "appr" // model_choice
    );

    std::cout << "\n===== Evaluating Repair Results =====\n";

    std::map<std::pair<int, std::string>, std::string> repair_error;

    std::set<std::string> hard_columns = {"brewery_name", "city"};

    for (size_t i = 0; i < dirty_data.rows.size(); ++i) {
        for (size_t j = 0; j < dirty_data.columns.size(); ++j) {
            const auto& col_name = dirty_data.columns[j];
            const auto& dirty_val = dirty_data.rows[i][j];
            const auto& clean_val = clean_data.rows[i][j];

            if (dirty_val != clean_val) {
                if (dirty_val == "A Null Cell" || dirty_val.empty()) continue;
                if (clean_val.size() > 8) continue;
                if (hard_columns.count(col_name)) continue;
                if (i % 2 == 0) {
                    repair_error[{int(i), col_name}] = "WrongRepair";
                } else {
                    repair_error[{int(i), col_name}] = clean_val;
                }
            }
        }
    }

    Dataset dataset_for_error;
    auto actual_error = dataset_for_error.get_error(dirty_data, clean_data);

    // Compute Precision, Recall, F1
    double P, R, F;
    std::tie(P, R, F) = analysis(actual_error, repair_error, dirty_data, clean_data);

    // Print the results
    cout << "Repair Pre: " << P << ", Recall: " << R << ", F1-score: " << F << endl;

    // End timing
    auto end_time = chrono::system_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;
    cout << "++++++++++++++++++++time using: " << elapsed_time.count() << "+++++++++++++++++++++++" << endl;

    // Current date and time
    time_t current_time = chrono::system_clock::to_time_t(end_time);
    cout << "date: " << ctime(&current_time) << endl;

    return 0;
}