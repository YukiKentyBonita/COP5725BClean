#ifndef INFERENCE_H
#define INFERENCE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <map>

// For Inference, we represent a row as an unordered_map from attribute names to values.
using RowMap = std::unordered_map<std::string, std::string>;
// And the dataset as a vector of such rows.
using DataFrameMap = std::vector<RowMap>;

// Placeholder types for model and compensative parameter.
// In a complete implementation, these would be replaced with proper classes.
using Model = int; // Placeholder for Bayesian network model.
using ModelDict = std::unordered_map<std::string, Model>;
using CompensativeParameterType = int; // Placeholder.

// For attribute type settings, assume a map from attribute name to a map of string settings.
using AttrType = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;

// FrequencyList and OccurrenceData are also placeholders.
using FrequencyList = std::unordered_map<std::string, std::unordered_map<std::string, double>>;
using OccurrenceData = std::unordered_map<std::string, 
                          std::unordered_map<std::string, 
                          std::unordered_map<std::string, 
                          std::unordered_map<std::string, double>>>>;

class Inference {
public:
    // Constructor: pass in dirtyData, observed data, model, model dictionary, attributes list,
    // frequency list, occurrence data, compensative parameter, and various strategy parameters.
    Inference(const DataFrameMap& dirtyData,
              const DataFrameMap& data,
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

    // Repair method – returns a repaired dataset.
    DataFrameMap Repair(const DataFrameMap& data, const DataFrameMap& cleanData,
                        const Model& model, const AttrType& attrType);

    // Helper: repairs a single row (simulating repair_line)
    RowMap repairLine(const RowMap& dataLine, int line,
                      const Model& modelAll,
                      const ModelDict& modelDict,
                      const std::vector<std::string>& nodeListSort,
                      const AttrType& attrType);

    // Pruning function: returns a list of attributes (nodes) that need repair.
    std::vector<std::string> prun(const RowMap& dataLine, int line,
                                  const AttrType& attrType,
                                  const std::vector<std::string>& nodeList);

private:
    DataFrameMap dirtyData;  // original dirty data
    DataFrameMap data;       // observed data
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

    // Dictionary to record repair errors; key format "line_attr"
    std::unordered_map<std::string, std::string> repairErr;
};

#endif  // INFERENCE_H