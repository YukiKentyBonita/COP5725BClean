#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <unordered_map>

// Include our module headers from the include directory.
#include "BayesianNetwork.h"
#include "Cleaner.h"
#include "Inference.h"

// For Cleaner, our dataset is represented as a vector of vector<string>
using DataFrameCleaner = std::vector<std::vector<std::string>>;

// For Inference, we represent a row as an unordered_map from attribute names to values.
using RowMap = std::unordered_map<std::string, std::string>;
using DataFrameMap = std::vector<RowMap>;

int main() {
    std::cout << "=== Testing BayesianNetwork ===" << std::endl;
    // Create a BayesianNetwork instance and add some dummy probabilities.
    BayesianNetwork bn;
    bn.addProbability("color", "red", 0.7);
    bn.addProbability("color", "blue", 0.3);
    
    double pRed = bn.getProbability("color", "red");
    double pBlue = bn.getProbability("color", "blue");
    double pGreen = bn.getProbability("color", "green");  // Not added; should return default value

    std::cout << "P(red): " << pRed << "\nP(blue): " << pBlue 
              << "\nP(green) (default): " << pGreen << std::endl;
    assert(std::abs(pRed - 0.7) < 1e-6);
    assert(std::abs(pBlue - 0.3) < 1e-6);
    assert(std::abs(pGreen - 1e-9) < 1e-12);
    std::cout << "BayesianNetwork tests passed!\n" << std::endl;
    
    std::cout << "=== Testing Cleaner ===" << std::endl;
    // Prepare a simple dataset for Cleaner.
    DataFrameCleaner dataset = { {"red"}, {"A Null Cell"} };
    std::vector<std::string> attributes = { "color" };
    
    // Create a Cleaner instance with the Bayesian network and dataset.
    Cleaner cleaner(bn, dataset, attributes);
    // Set the candidate domain for "color".
    cleaner.setDomain("color", {"red", "blue"});
    
    auto cleanedData = cleaner.cleanData();
    
    std::cout << "Cleaner output:" << std::endl;
    for (const auto &row : cleanedData) {
        for (const auto &val : row) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }
    // Expect the first row to remain "red" and the second to be replaced by "CorrectedValue" (per our dummy logic).
    assert(cleanedData[0][0] == "red");
    assert(cleanedData[1][0] == "CorrectedValue");
    std::cout << "Cleaner tests passed!\n" << std::endl;
    
    std::cout << "=== Testing Inference ===" << std::endl;
    // Prepare a simple dataset for Inference (using map-based rows).
    DataFrameMap dirtyDataMap = { { {"color", "A Null Cell"} } };
    DataFrameMap observedDataMap = dirtyDataMap;  // For testing, use the same data.
    
    // Set up dummy parameters for Inference.
    int dummyModel = 0;  // Dummy placeholder.
    std::unordered_map<std::string, int> dummyModelDict; // Empty dictionary.
    std::vector<std::string> inferAttrs = { "color" };
    FrequencyList dummyFreqList;   // Empty placeholder.
    OccurrenceData dummyOccData;   // Empty placeholder.
    int dummyCompParam = 0;        // Dummy compensative parameter.
    
    // Create a dummy attribute type map.
    // For "color", we mark it as repairable ("Y"), with an empty pattern, and disallow null values.
    AttrType attrType;
    attrType["color"] = { {"repairable", "Y"}, {"pattern", ""}, {"AllowNull", "N"} };
    
    Inference inference(dirtyDataMap, observedDataMap, dummyModel, dummyModelDict, 
                        inferAttrs, dummyFreqList, dummyOccData, dummyCompParam, 
                        "PIPD", 1, 1, 1.0);
    
    DataFrameMap repairedData = inference.Repair(observedDataMap, observedDataMap, dummyModel, attrType);
    
    std::cout << "Inference output (repaired data):" << std::endl;
    for (const auto &row : repairedData) {
        for (const auto &kv : row) {
            std::cout << kv.first << ": " << kv.second << " ";
        }
        std::cout << std::endl;
    }
    // According to our dummy repair logic in Inference, "A Null Cell" should become "CorrectedValue".
    assert(repairedData[0]["color"] == "CorrectedValue");
    std::cout << "Inference tests passed!" << std::endl;
    
    std::cout << "\nAll my part tests passed successfully!" << std::endl;
    return 0;
}