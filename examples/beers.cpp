#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <thread>
#include "../dataset.h"
#include "../include/UserConstraints.h"  // Include UC class header
using namespace std;

int main() {
    Dataset dataset;
    // Paths to the dirty and clean data
    string dirty_path = "/Users/yukikentybonita/Desktop/FSU/COP5725BClean/data/dirty.csv";
    string clean_path = "/Users/yukikentybonita/Desktop/FSU/COP5725BClean/data/clean.csv";
    string json_path = "/Users/yukikentybonita/Desktop/FSU/COP5725BClean/json/beers.json";
    
    // Load data using the Dataset methods.
    DataFrame dirty_data = dataset.get_data(dirty_path);
    DataFrame clean_data = dataset.get_data(clean_path);

    //=================debug for dataset.cpp======================
    // // Print the dirty data
    // cout << "Dirty Data:" << endl;
    // // Print column headers
    // for (const auto& col : dirty_data.columns) {
    //     cout << col << "\t";
    // }
    // cout << endl;

    // for (const auto& row : dirty_data.rows) {
    //     for (const auto& cell : row) {
    //         cout << cell << "\t";
    //     }
    //     cout << endl;
    // }

    // // Similarly, print the clean data (optional debugging step)
    // cout << "\nClean Data:" << endl;
    // for (const auto& col : clean_data.columns) {
    //     cout << col << "\t";
    // }
    // cout << endl;

    // for (const auto& row : clean_data.rows) {
    //     for (const auto& cell : row) {
    //         cout << cell << "\t";
    //     }
    //     cout << endl;
    // }
    //=================debug for dataset.cpp======================

    
    // // Initialize UC (User Constraints) with dirty data
    UC uc(dirty_data);
    // **Calling build_from_json()**
    cout << "Loading constraints from JSON file..." << endl;
    uc.build_from_json(json_path);  // Call build_from_json to load constraints from the JSON file

    // **Print out the content of the JSON file (loaded into UC)**
    // Print out the content of the JSON file (loaded into UC)
    cout << "User Constraints after loading from JSON:" << endl;
    auto updated_uc_data = uc.get_uc();
    for (const auto& [key, value] : updated_uc_data) {
        cout << key << " : ";
        for (const auto& [attr, val] : value) {
            cout << attr << "=" << val << " ";
        }
        cout << endl;
    }

    // // Build user constraints for each attribute in the dirty data
    // for (const auto& attr : dirty_data.columns) {
    //     // Here, you can modify the type or values based on the attribute
    //     uc.build(attr, "Categorical", "min_value", "max_value");  // Example for categorical
    //     // You can also use specific types for different attributes based on your dataset
    // }

    // // Print the user constraints for each attribute
    // auto uc_data = uc.get_uc();
    // for (const auto& [key, value] : uc_data) {
    //     cout << key << " : ";
    //     for (const auto& [attr, val] : value) {
    //         cout << attr << "=" << val << " ";
    //     }
    //     cout << endl;
    // }

    // Placeholder for applying pattern discovery (this will use UC functionality)
    cout << "Applying Pattern Discovery..." << endl;
    auto patterns = uc.PatternDiscovery();  // Use UC's pattern discovery method
    cout << "Pattern discovery: ";
    for (const auto& pat : patterns) {
        cout << pat.first << ": " << pat.second << " ";
    }
    cout << endl;

    // // Start timing the BayesianClean process
    // auto start_time = chrono::high_resolution_clock::now();

    // Simulating repair data
    map<string, AttrInfo> attr_type;
    for (const auto& [col, constraints] : updated_uc_data) {
        if (constraints.find("type") != constraints.end()) {
            attr_type[col] = {constraints.at("type")};  // Extract type from UC
        } else {
            attr_type[col] = {"Unknown"};  // Ensure all attributes are included
        }
    }

    // **Call get_real_data() correctly**
    dirty_data = dataset.get_real_data(dirty_data, attr_type);
    clean_data = dataset.get_real_data(dirty_data, attr_type);

    // // **Print cleaned DataFrame**
    // cout << "\nFiltered DataFrame after applying UC constraints:\n";
    // dataset.print_dataframe(dirty_data);

    // // Simulate the actual and repaired error calculations
    // double actual_error = 0.05;  // Placeholder for actual error calculation
    // double repair_error = 0.02;  // Placeholder for repair error calculation

    // // Placeholder for Precision, Recall, F1-score calculation
    // double P = 0.9, R = 0.85, F = 2 * (P * R) / (P + R);  // Example values for evaluation
    // cout << "Repair Pre: " << P << ", Recall: " << R << ", F1-score: " << F << endl;

    // // End timing the process
    // auto end_time = chrono::high_resolution_clock::now();
    // chrono::duration<double> elapsed_time = end_time - start_time;

    // // Display elapsed time
    // cout << "++++++++++++++++++++time using: " << elapsed_time.count() << "+++++++++++++++++++++++" << endl;

    // // Print the current date and time
    // time_t current_time = chrono::system_clock::to_time_t(end_time);
    // cout << "date: " << ctime(&current_time) << endl;

    return 0;
}