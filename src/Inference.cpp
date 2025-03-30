#include "../include/Inference.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>

// Constructor implementation
Inference::Inference(const DataFrame& dirtyData,
                     const DataFrame& data,
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
    // Initialization: you may initialize your compensative parameter, etc.
    std::cout << "Inference module initialized with strategy: " << inferStrategy << std::endl;
}

// A simplified version of the Repair method.
DataFrame Inference::Repair(const DataFrame& data, const DataFrame& cleanData,
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
    DataFrame dirtyDataCopy = data;
    // Fill missing values with "A Null Cell"
    for (auto& row : dirtyDataCopy) {
        for (const auto& node : nodesList) {
            if (row.find(node) == row.end() || row[node].empty())
                row[node] = "A Null Cell";
        }
    }

    DataFrame repairData = dirtyDataCopy;
    DataFrame cleanDataCopy = cleanData; // assuming similar treatment

    // For simplicity, we assume nodesListSort is the same as nodesList.
    std::vector<std::string> nodesListSort = nodesList;
    // (In a full implementation, sort nodesListSort by number of parents in the BN.)

    // Initialize compensative parameter (placeholder)
    // compParam.init_tf_idf(nodesListSort);

    int maxIter = 1;
    int totalRows = repairData.size();
    for (int iter = 0; iter < maxIter; iter++) {
        for (int line = 0; line < totalRows; line++) {
            // Here we call repairLine for each row.
            Row repairedRow = repairLine(repairData[line], line, model, modelDict, nodesListSort, attrType);
            // Replace the row in repairData with repairedRow.
            repairData[line] = repairedRow;

            // In a complete implementation, update repairErr from repairLine as well.
            if ((line + 1) % 100 == 0) {
                std::cout << (line + 1) << " tuple repairing done" << std::endl;
            }
        }
        std::cout << "++++++++++++++++The " << (iter + 1) << "/" << maxIter << " Iteration+++++++++++++++++" << std::endl;
    }

    // For simplicity, return the repaired data.
    return repairData;
}

// A simplified repair_line function.
// This function should compute candidate scores, compare them, and update the row accordingly.
// Here we provide a dummy implementation that returns the input row unchanged.
Row Inference::repairLine(const Row& dataLine, int line,
                          const Model& modelAll,
                          const ModelDict& modelDict,
                          const std::vector<std::string>& nodeListSort,
                          const AttrType& attrType)
{
    // Make a copy of the row to repair.
    Row repairedRow = dataLine;

    // Simulate pruning: call prun() to get attributes needing repair.
    std::vector<std::string> repairNodes = prun(dataLine, line, attrType, nodeListSort);

    // For each attribute that needs repair, choose a candidate value.
    // (In a full implementation, you would compute probabilities using the BN,
    // compensatory scores, and then pick the candidate with the highest score.)
    for (const std::string& attr : repairNodes) {
        // Dummy candidate selection: if current value is "A Null Cell", replace it with "CorrectedValue"
        if (dataLine.at(attr) == "A Null Cell") {
            repairedRow[attr] = "CorrectedValue";
            // Record the repair error (key format: "line_attr")
            std::ostringstream oss;
            oss << line << "_" << attr;
            repairErr[oss.str()] = "CorrectedValue";
        }
    }

    return repairedRow;
}

// A simplified prun method that returns attributes needing repair.
// In the original Python code, it computes a score based on co-occurrence and frequency.
// Here we simulate by returning all attributes whose value equals "A Null Cell".
std::vector<std::string> Inference::prun(const Row& dataLine, int line,
                                        const AttrType& attrType,
                                        const std::vector<std::string>& nodeList)
{
    std::vector<std::string> res;
    for (const std::string& attr : nodeList) {
        auto it = dataLine.find(attr);
        if (it != dataLine.end()) {
            if (it->second == "A Null Cell") { // or any condition for low confidence
                res.push_back(attr);
            }
        }
    }
    return res;
}