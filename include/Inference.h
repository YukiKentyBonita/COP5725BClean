#ifndef INFERENCE_H
#define INFERENCE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <map>

// For simplicity, we represent a row of the dataset as a map from attribute names to values.
using Row = std::unordered_map<std::string, std::string>;
// And a DataFrame as a vector of rows.
using DataFrame = std::vector<Row>;

// Placeholder types for model and compensative parameter.
// In a complete implementation, these would be classes representing your BN, CPD functions, etc.
using Model = int; // Replace with your Bayesian network model class
using ModelDict = std::unordered_map<std::string, Model>;
using CompensativeParameterType = int; // Replace with your actual type

// For attribute type settings, we assume a map from attribute name to a map of string settings.
using AttrType = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;

// Frequency list and occurrence data are complex nested maps.
// Here we use a simplified placeholder.
using FrequencyList = std::unordered_map<std::string, std::unordered_map<std::string, double>>;
using OccurrenceData = std::unordered_map<std::string, 
                          std::unordered_map<std::string, 
                          std::unordered_map<std::string, 
                          std::unordered_map<std::string, double>>>>;

class Inference {
public:
    // Constructor: pass in dirty_data, observed data, model, model_dict, attributes list, frequency list, occurrence data,
    // compensative parameter, inference strategy, chunk size, number of workers, and tuple pruning threshold.
    Inference(const DataFrame& dirtyData,
              const DataFrame& data,
              const Model& model,
              const ModelDict& modelDict,
              const std::vector<std::string>& attrs,
              const FrequencyList& freqList,
              const OccurrenceData& occData,
              const CompensativeParameterType& compParam,
              const std::string& inferStrategy = "PIPD",
              int chunkSize = 1,
              int numWorker = 1,
              double tuplePrun = 1.0);

    // Repair method – returns a repaired DataFrame.
    DataFrame Repair(const DataFrame& data, const DataFrame& cleanData,
                     const Model& model,
                     const AttrType& attrType);

    // Helper: repairs a single row (simulating repair_line)
    Row repairLine(const Row& dataLine, int line,
                   const Model& modelAll,
                   const ModelDict& modelDict,
                   const std::vector<std::string>& nodeListSort,
                   const AttrType& attrType);

    // Pruning function: returns a list of attributes (nodes) that need repair
    std::vector<std::string> prun(const Row& dataLine, int line,
                                  const AttrType& attrType,
                                  const std::vector<std::string>& nodeList);

private:
    DataFrame dirtyData;  // original dirty data
    DataFrame data;       // observed data
    Model model;          // main Bayesian network model (placeholder)
    ModelDict modelDict;  // additional models keyed by attribute
    std::vector<std::string> attrs;  // list of attribute names
    FrequencyList freqList;          // frequency list for compensative scoring
    OccurrenceData occData;          // occurrence data for compensative scoring
    CompensativeParameterType compParam; // compensative parameter (placeholder)
    std::string inferStrategy;
    int chunkSize;
    int numWorker;
    double tuplePrun;

    // Dictionary to record repair errors; key could be "line_attr"
    std::unordered_map<std::string, std::string> repairErr;
};

#endif  // INFERENCE_H