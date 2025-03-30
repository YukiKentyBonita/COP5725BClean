#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>
#include "../include/BayesianNetwork.h"
#include "../include/Cleaner.h"
#include "../include/Inference.h"

// For Inference, we use these type aliases:
using RowMap = std::unordered_map<std::string, std::string>;
using DataFrameMap = std::vector<RowMap>;

int main() {
    std::cout << "=== Testing BayesianNetwork ===" << std::endl;
    // --- Test BayesianNetwork ---
    BayesianNetwork bn;
    bn.addProbability("state", "NY", 0.6);
    bn.addProbability("state", "CA", 0.4);
    
    double pNY = bn.getProbability("state", "NY");
    double pCA = bn.getProbability("state", "CA");
    double pTX = bn.getProbability("state", "TX"); // Not added; should return default value
    
    std::cout << "P(NY): " << pNY << "\nP(CA): " << pCA 
              << "\nP(TX): " << pTX << std::endl;
    assert(std::abs(pNY - 0.6) < 1e-6);
    assert(std::abs(pCA - 0.4) < 1e-6);
    assert(std::abs(pTX - 1e-9) < 1e-12);
    std::cout << "BayesianNetwork tests passed!\n" << std::endl;
    
    std::cout << "=== Testing Cleaner ===" << std::endl;
    // --- Test Cleaner ---
    // Using a simple dataset with one attribute "state".
    // In Cleaner, our DataFrame is a vector of vector<string>.
    std::vector<std::vector<std::string>> dataset = { {"NY"}, {"A Null Cell"} };
    std::vector<std::string> attributes = { "state" };
    
    // Create a Cleaner instance with our Bayesian network and dataset.
    Cleaner cleaner(bn, dataset, attributes);
    // Set the candidate domain for "state"
    cleaner.setDomain("state", {"NY", "CA"});
    
    auto cleanedData = cleaner.cleanData();
    
    std::cout << "Cleaner output:" << std::endl;
    for (const auto &row : cleanedData) {
        for (const auto &val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
    // Our dummy repair logic in Cleaner (in repairLine) is set to replace "A Null Cell" with "CorrectedValue"
    assert(cleanedData[0][0] == "NY");
    assert(cleanedData[1][0] == "CorrectedValue");
    std::cout << "Cleaner tests passed!\n" << std::endl;
    
    std::cout << "=== Testing Inference ===" << std::endl;
    // --- Test Inference ---
    // For Inference, our DataFrame is represented as vector of RowMap (unordered_map<string, string>)
    DataFrameMap dirtyDataMap = { { {"state", "A Null Cell"} } };
    DataFrameMap observedDataMap = dirtyDataMap; // For testing, use same data.
    
    // Dummy model: we use an int as a placeholder.
    int dummyModel = 0;
    // Dummy model dictionary: empty map
    std::unordered_map<std::string, int> dummyModelDict;
    // Attributes list (for Inference, we use the same attribute "state")
    std::vector<std::string> attrs = { "state" };
    // Dummy frequency list and occurrence data (empty for now)
    FrequencyList dummyFreqList;
    OccurrenceData dummyOccData;
    // Dummy compensative parameter (using int placeholder)
    int dummyCompParam = 0;
    
    // Create a dummy attribute type map.
    // For "state", we mark it as repairable ("Y"), with an empty pattern and AllowNull "N".
    AttrType attrType;
    attrType["state"] = { {"repairable", "Y"}, {"pattern", ""}, {"AllowNull", "N"} };
    
    // Create an Inference instance with our dummy values.
    Inference inference(dirtyDataMap, observedDataMap, dummyModel, dummyModelDict, 
                        attrs, dummyFreqList, dummyOccData, dummyCompParam, 
                        "PIPD", 1, 1, 1.0);
    
    // Call the Repair method. (In our skeleton, it uses dummy repair logic.)
    DataFrameMap repairedData = inference.Repair(observedDataMap, observedDataMap, dummyModel, attrType);
    
    std::cout << "Inference output (repaired data):" << std::endl;
    for (const auto& row : repairedData) {
        for (const auto& kv : row) {
            std::cout << kv.first << ": " << kv.second << " ";
        }
        std::cout << std::endl;
    }
    
    // In our dummy repairLine, if the value equals "A Null Cell", it is replaced with "CorrectedValue".
    // So we expect the "state" attribute in the first row to become "CorrectedValue".
    assert(repairedData[0]["state"] == "CorrectedValue");
    
    std::cout << "Inference tests passed!" << std::endl;
    std::cout << "All module tests passed successfully!" << std::endl;
    
    return 0;
}