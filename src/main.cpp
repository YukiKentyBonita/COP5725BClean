#include <iostream>
#include "../dataset.h"
#include "../include/UserConstraints.h"
#include "BayesianNetwork.h"  // our custom BN module
#include "Cleaner.h"          // our custom Cleaner module
#include "Inference.h"        // our custom Inference module

using namespace std;

int main() {
    // Load data
    Dataset dataset;
    string dirty_path = "data/dirty.csv";
    string clean_path = "data/clean.csv";
    string json_path = "json/beers.json";
    DataFrame dirty_data = dataset.get_data(dirty_path);
    DataFrame clean_data = dataset.get_data(clean_path);

    // Load and apply user constraints
    UC uc(dirty_data);
    cout << "Loading constraints from JSON file..." << endl;
    uc.build_from_json(json_path);
    auto updated_uc_data = uc.get_uc();
    // Print out constraints (for debugging)
    for (const auto& [key, value] : updated_uc_data) {
        cout << key << " : ";
        for (const auto& [attr, val] : value) {
            cout << attr << "=" << val << " ";
        }
        cout << endl;
    }

    // Pattern discovery, etc.
    cout << "Applying Pattern Discovery..." << endl;
    auto patterns = uc.PatternDiscovery();
    // Print patterns
    for (const auto &pat : patterns) {
        cout << pat.first << ": " << pat.second << " ";
    }
    cout << endl;

    // Transform data using get_real_data and pre_process_data
    dirty_data = dataset.get_real_data(dirty_data, /* attr_type from updated_uc_data */);
    clean_data = dataset.get_real_data(clean_data, /* same attr_type */);
    DataFrame preprocessed_data = dataset.pre_process_data(dirty_data, /* attr_type */);

    // (Optional) Print preprocessed data for verification

    // Now, set up your Bayesian cleaning modules.
    // For instance, build a Bayesian network from the preprocessed_data.
    BayesianNetwork bn;
    // (Populate bn with probabilities based on your data or use a learning algorithm.)

    // You can now test the Cleaner module.
    // Convert your preprocessed_data to the format expected by Cleaner (DataFrameCleaner)
    DataFrameCleaner dataForCleaner = convertToCleanerFormat(preprocessed_data);
    vector<string> attributes = preprocessed_data.columns;  // assuming you have a columns vector
    Cleaner cleaner(bn, dataForCleaner, attributes);
    // Set candidate domains for attributes (e.g., based on your data statistics)
    cleaner.setDomain("state", {"NY", "CA", "TX"});  // example

    DataFrameCleaner cleanedData = cleaner.cleanData();
    // Print the cleaned data.

    // Alternatively, test the Inference module if that is your desired pipeline.
    // Convert preprocessed_data to DataFrameMap format for Inference.
    DataFrameMap dataForInference = convertToInferenceFormat(preprocessed_data);
    // Set up a dummy model and attribute types as needed.
    int dummyModel = 0;
    unordered_map<string, int> dummyModelDict;
    vector<string> inferAttrs = attributes;
    FrequencyList dummyFreqList;
    OccurrenceData dummyOccData;
    int dummyCompParam = 0;
    AttrType attrType;
    // Populate attrType based on updated_uc_data or set defaults.
    for (const auto& col : attributes) {
        attrType[col] = { {"repairable", "Y"}, {"pattern", ""}, {"AllowNull", "N"} };
    }
    Inference inference(dataForInference, dataForInference, dummyModel, dummyModelDict,
                        inferAttrs, dummyFreqList, dummyOccData, dummyCompParam,
                        "PIPD", 1, 1, 1.0);
    DataFrameMap repairedData = inference.Repair(dataForInference, dataForInference, dummyModel, attrType);
    // Print repaired data and compute error metrics against clean_data if available.

    return 0;
}