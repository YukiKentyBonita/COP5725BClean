#ifndef COMPENSATIVEPARAMETER_H
#define COMPENSATIVEPARAMETER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include "dataset.h"      // For DataFrame, Row, AttrInfo
#include "BNStructure.h"  // For BNGraph

using std::string;
using std::vector;
using std::unordered_map;
using std::map;

// An alias for a row in the DataFrame
using Row = unordered_map<string, string>;

// Penalty computation
class CompensativeParameter {
public:
    CompensativeParameter(const map<string, AttrInfo>& attr_type,
                          const unordered_map<string, unordered_map<string, int>>& domain,
                          const unordered_map<string,
                              unordered_map<string,
                                  unordered_map<string,
                                      unordered_map<string, double>>>>& occurrence,
                          const BNGraph& model,
                          const DataFrame& df);

    // Compute penalty scores for a given observed value (obs) for attribute (attr)
    unordered_map<string, double> return_penalty(const string& obs,
                                                   const string& attr,
                                                   int index,
                                                   const Row& data_line,
                                                   const vector<string>& prior);

    // TF-IDF–based variant for penalty scoring
    unordered_map<string, double> return_penalty_test(const string& obs,
                                                      const string& attr,
                                                      int index,
                                                      const Row& data_line,
                                                      const vector<string>& prior,
                                                      const vector<string>& attr_order);

    // Initialization of TF-IDF data
    void init_tf_idf(const vector<string>& attr_order);

private:
    map<string, AttrInfo> attr_type;
    unordered_map<string, unordered_map<string, int>> domain;

    // Weighted co-occurrence counts
    unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, double>>>> occurrence;
    // BN model
    BNGraph model;

    // Dataset
    DataFrame df;

    // Data structure to hold TF-IDF info
    struct TFIDFData {
        vector<string> combine_attrs;                   
        unordered_map<string, int> dic;   
        unordered_map<string, int> dic_idf;
    };

    // Map from an attribute name to its TF-IDF data
    unordered_map<string, std::shared_ptr<TFIDFData>> tf_idf;

    // ----------------- Helper Functions -----------------

    // Compute Levenshtein distance (edit distance) between two strings
    int levenshtein_distance(const string& s1, const string& s2);

    // Compute Euclidean (L2) norm of a vector
    double euclidean_norm(const vector<double>& vec);
};

#endif // COMPENSATIVEPARAMETER_H