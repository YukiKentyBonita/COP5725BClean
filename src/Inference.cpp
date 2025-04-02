#include "../include/Inference.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>

// Constructor implementation
Inference::Inference(const DataFrameMap& dirtyData,
                     const DataFrameMap& data,
                     const Model& model,
                     const ModelDict& modelDict,
                     const std::vector<std::string>& attrs,
                     const FrequencyList& freqList,
                     const OccurrenceData& occData,
                     const CompensativeParameterType& compParam,
                     const std::string& inferStrategy,
                     int chunkSize,
                     int numWorker,
                     double tuplePrun)
    : dirtyData(dirtyData), data(data), model(model), modelDict(modelDict),
      attrs(attrs), freqList(freqList), occData(occData),
      compParam(compParam), inferStrategy(inferStrategy),
      chunkSize(chunkSize), numWorker(numWorker), tuplePrun(tuplePrun)
{
    std::cout << "Inference module initialized with strategy: " << inferStrategy << std::endl;
}

// The Repair method returns a repaired DataFrameMap.
DataFrameMap Inference::Repair(const DataFrameMap& data, const DataFrameMap& cleanData,
                               const Model& model,
                               const AttrType& attrType)
{
    std::cout << "++++++++++++++++++++++++error repairing+++++++++++++++++++++++++++++" << std::endl;
    
    // Create a list of nodes from attrType keys.
    std::vector<std::string> nodesList;
    for (const auto& pair : attrType) {
        nodesList.push_back(pair.first);
    }

    // Create a copy of the observed data for the nodes of interest.
    DataFrameMap dirtyDataCopy = data;
    // Fill missing values with "A Null Cell" for each node.
    for (auto& row : dirtyDataCopy) {
        for (const auto& node : nodesList) {
            if (row.find(node) == row.end() || row[node].empty())
                row[node] = "A Null Cell";
        }
    }

    DataFrameMap repairData = dirtyDataCopy;
    DataFrameMap cleanDataCopy = cleanData; // assuming similar treatment

    // For simplicity, assume nodesListSort is the same as nodesList.
    std::vector<std::string> nodesListSort = nodesList;

    // Placeholder: initialize compensative parameter if needed.
    // compParam.init_tf_idf(nodesListSort);

    int maxIter = 1;
    int totalRows = repairData.size();
    for (int iter = 0; iter < maxIter; iter++) {
        for (int line = 0; line < totalRows; line++) {
            // Call repairLine for each row.
            RowMap repairedRow = repairLine(repairData[line], line, model, modelDict, nodesListSort, attrType);
            repairData[line] = repairedRow;
            if ((line + 1) % 100 == 0) {
                std::cout << (line + 1) << " tuple repairing done" << std::endl;
            }
        }
        std::cout << "++++++++++++++++The " << (iter + 1) << "/" << maxIter << " Iteration+++++++++++++++++" << std::endl;
    }
    return repairData;
}

// repairLine: Repairs a single row of type RowMap.
RowMap Inference::repairLine(const RowMap& dataLine, int line,
                             const Model& modelAll,
                             const ModelDict& modelDict,
                             const std::vector<std::string>& nodeListSort,
                             const AttrType& attrType)
{
    // Make a copy of the row to repair.
    RowMap repairedRow = dataLine;

    // Simulate pruning: get attributes that need repair.
    std::vector<std::string> repairNodes = prun(dataLine, line, attrType, nodeListSort);

    // For each attribute that needs repair, choose a candidate value.
    // Dummy logic: if the current value is "A Null Cell", replace it with "CorrectedValue".
    for (const std::string& attr : repairNodes) {
        if (dataLine.at(attr) == "A Null Cell") {
            repairedRow[attr] = "CorrectedValue";
            std::ostringstream oss;
            oss << line << "_" << attr;
            repairErr[oss.str()] = "CorrectedValue";
        }
    }
    return repairedRow;
}

// prun: Returns a vector of attributes (from nodeList) whose value equals "A Null Cell".
std::vector<std::string> Inference::prun(const RowMap& dataLine, int line,
                                         const AttrType& attrType,
                                         const std::vector<std::string>& nodeList)
{
    std::vector<std::string> res;
    for (const std::string& attr : nodeList) {
        auto it = dataLine.find(attr);
        if (it != dataLine.end()) {
            if (it->second == "A Null Cell") { // Condition for needing repair
                res.push_back(attr);
            }
        }
    }
    return res;
}