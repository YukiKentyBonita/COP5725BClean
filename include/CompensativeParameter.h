#ifndef COMPENSATIVEPARAMETER_H
#define COMPENSATIVEPARAMETER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include "dataset.h"      // For DataFrame, Row, AttrInfo
#include "BNStructure.h"  // For BNGraph

// Using declarations for convenience:
using std::string;
using std::vector;
using std::unordered_map;
using std::map;

// An alias for a row in the DataFrame.
using Row = unordered_map<string, string>;

// The CompensativeParameter class implements the penalty computation functions,
// which take as input (1) the attribute info, (2) the frequency/domain information,
// (3) the occurrence (weighted co-occurrence counts), (4) the Bayesian Network model,
// and (5) the original dataset.
class CompensativeParameter {
public:
    // Constructor:
    //   - attr_type: attribute meta-data (e.g. rules for AllowNull and patterns)
    //   - domain: frequency list (e.g. from Compensative::getFrequencyList())
    //   - occurrence: co-occurrence info (from Compensative::getOccurrenceList())
    //   - model: the BNGraph from BNStructure (bn_result.model)
    //   - df: the full data frame (DataFrame)
    CompensativeParameter(const map<string, AttrInfo>& attr_type,
                          const unordered_map<string, unordered_map<string, int>>& domain,
                          const unordered_map<string,
                              unordered_map<string,
                                  unordered_map<string,
                                      unordered_map<string, double>>>>& occurrence,
                          const BNGraph& model,
                          const DataFrame& df);

    // Compute penalty scores for a given observed value (obs) for attribute (attr).
    // The index is the row index in the data, data_line is a single row,
    // and prior is a vector of candidate (ground) values.
    unordered_map<string, double> return_penalty(const string& obs,
                                                   const string& attr,
                                                   int index,
                                                   const Row& data_line,
                                                   const vector<string>& prior);

    // A TF-IDF–based variant for penalty scoring. attr_order is used to build context.
    unordered_map<string, double> return_penalty_test(const string& obs,
                                                      const string& attr,
                                                      int index,
                                                      const Row& data_line,
                                                      const vector<string>& prior,
                                                      const vector<string>& attr_order);

    // Initialization of TF-IDF data (computes context statistics based on attr_order).
    void init_tf_idf(const vector<string>& attr_order);

private:
    // Member variables from the Python __init__:
    map<string, AttrInfo> attr_type;
    // "domain" is the frequency list
    unordered_map<string, unordered_map<string, int>> domain;
    // The weighted co-occurrence counts.
    unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, double>>>> occurrence;
    // The BN model.
    BNGraph model;
    // The dataset (a DataFrame, assumed to be a vector of Row)
    DataFrame df;

    // Data structure to hold TF-IDF info for one attribute.
    struct TFIDFData {
        vector<string> combine_attrs;                   // The names of attributes used for context.
        unordered_map<string, int> dic;                 // “Term frequency” of the combined context.
        unordered_map<string, int> dic_idf;             // Frequency counts of the attribute values.
    };

    // Map from an attribute name to its TF-IDF data (or nullptr if none is computed).
    unordered_map<string, std::shared_ptr<TFIDFData>> tf_idf;

    // ----------------- Helper Functions -----------------

    // Compute Levenshtein distance (edit distance) between two strings.
    int levenshtein_distance(const string& s1, const string& s2);

    // Compute Euclidean (L2) norm of a vector.
    double euclidean_norm(const vector<double>& vec);
};

#endif // COMPENSATIVEPARAMETER_H