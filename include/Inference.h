// include/Inference.h
#ifndef INFERENCE_H
#define INFERENCE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include "dataset.h"                // for DataFrame, AttrInfo
#include "CompensativeParameter.h"  // for CompensativeParameter
#include "BNStructure.h"            // for BNGraph

using std::string;
using std::vector;
using std::unordered_map;
using std::map;
using std::shared_ptr;

// A single row is an attr→value map
using Row     = unordered_map<string, string>;
// A full dataset is a vector of rows
using DataMap = vector<Row>;
// Metadata for attributes
using AttrType = map<string, AttrInfo>;

class Inference {
public:
    Inference(const DataMap&                                      dirtyData,
              const DataMap&                                      processedData,
              const BNGraph&                                      fullGraph,
              const unordered_map<string,BNGraph>&                modelDict,
              const AttrType&                                     attrType,
              const unordered_map<string, unordered_map<string,int>>& frequencyList,
              const unordered_map<string,
                    unordered_map<string,
                        unordered_map<string,
                            unordered_map<string,int>>>>&         occurrence1,
              const shared_ptr<CompensativeParameter>&            compParam,
              const string&                                       inferStrategy = "PIPD",
              int                                                 chunkSize     = 1,
              int                                                 numWorker     = 1,
              double                                              tuplePrun     = 1.0,
              bool                                                debug         = false);

    // Run repair over all rows
    DataMap repair(const DataMap&  data,
                   const DataFrame& cleanData,
                   const BNGraph&   fullGraph,
                   const AttrType&  attrType);

private:
    Row repairLine(const Row&                       dataLine,
                   int                                line,
                   const BNGraph&                     fullGraph,
                   const unordered_map<string,BNGraph>& modelDict,
                   const vector<string>&              nodeList,
                   const AttrType&                    attrType);

    vector<string> prun(const Row&              dataLine,
                        int                     line,
                        const AttrType&         attrType,
                        const vector<string>&   nodeList);

    // members
    DataMap                                             dirtyData_;
    DataMap                                             data_;
    BNGraph                                             model_;
    unordered_map<string,BNGraph>                       modelDict_;
    AttrType                                            attrType_;
    unordered_map<string, unordered_map<string,int>>    frequencyList_;
    unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string,int>>>>             occurrence1_;
    shared_ptr<CompensativeParameter>                   compParam_;
    string                                              inferStrategy_;
    int                                                 chunkSize_;
    int                                                 numWorker_;
    double                                              tuplePrun_;
    bool                                                debug_;
    unordered_map<string,string>                        repairErr_;
};

#endif // INFERENCE_H