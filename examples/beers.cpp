#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <thread>
#include "../dataset.h"
using namespace std;

int main() {
    Dataset dataset;
    // Paths to the dirty and clean data
    string dirty_path = "/home/zsong/COP5725BClean/data/dirty.csv";
    string clean_path = "/home/zsong/COP5725BClean/data/clean.csv";
    
    // Load data using the Dataset methods.
    DataFrame dirty_data = dataset.get_data(dirty_path);
    DataFrame clean_data = dataset.get_data(clean_path);

    // Print the dirty data
    cout << "Dirty Data:" << endl;
    // Print column headers
    for (const auto& col : dirty_data.columns) {
        cout << col << "\t";
    }
    cout << endl;

    // Print each row of data
    /*for (const auto& row : dirty_data.rows) {
        for (const auto& cell : row) {
            cout << cell << "\t";
        }
        cout << endl;
    }

    // Similarly, print the clean data
    cout << "\nClean Data:" << endl;
    for (const auto& col : clean_data.columns) {
        cout << col << "\t";
    }
    cout << endl;

    for (const auto& row : clean_data.rows) {
        for (const auto& cell : row) {
            cout << cell << "\t";
        }
        cout << endl;
    }*/
    
    // Placeholder for UC class initialization and processing
    vector<string> attr_type;
    attr_type.push_back("attr_type_placeholder");

    // Placeholder for applying pattern discovery
    vector<string> pattern_discovery = {"pattern1", "pattern2"};  // Example patterns
    cout << "pattern discovery: ";
    for (const auto& pat : pattern_discovery) {
        cout << pat << " ";
    }
    cout << endl;

    // Start timing the BayesianClean process
    auto start_time = chrono::high_resolution_clock::now();

    // Since dirty_data is a DataFrame, we use its 'rows' member.
    vector<vector<string>> repaired_data = dirty_data.rows;          // Placeholder for repaired data
    vector<vector<string>> intermediate_repair = dirty_data.rows;      // Placeholder for intermediate repair data
    vector<vector<string>> actual_clean_data = clean_data.rows;        // Placeholder for actual clean data

    // Simulate the actual and repaired error calculations
    double actual_error = 0.05;  // Placeholder for actual error calculation
    double repair_error = 0.02;  // Placeholder for repair error calculation

    // Placeholder for Precision, Recall, F1-score calculation
    double P = 0.9, R = 0.85, F = 2 * (P * R) / (P + R);  // Example values for evaluation
    cout << "Repair Pre: " << P << ", Recall: " << R << ", F1-score: " << F << endl;

    // End timing the process
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    // Display elapsed time
    cout << "++++++++++++++++++++time using: " << elapsed_time.count() << "+++++++++++++++++++++++" << endl;

    // Print the current date and time
    time_t current_time = chrono::system_clock::to_time_t(end_time);
    cout << "date: " << ctime(&current_time) << endl;

    return 0;
}