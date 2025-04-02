#ifndef CLEANER_H
#define CLEANER_H

#include <vector>
#include <string>
#include <map>
#include "BayesianNetwork.h"  // Include full definition of BayesianNetwork

// Define a simple alias for our dataset used in the Cleaner module.
// A row for cleaning is represented as a vector of strings.
using RowCleaner = std::vector<std::string>;
// The dataset (DataFrameCleaner) is a vector of rows.
using DataFrameCleaner = std::vector<RowCleaner>;

class Cleaner {
private:
    // Reference to the Bayesian network (assumed to be already constructed)
    BayesianNetwork& bn;
    // The dataset to be cleaned.
    DataFrameCleaner data;
    // List of attribute names corresponding to each column.
    std::vector<std::string> attributes;
    // Domain of possible candidate values for each attribute.
    std::map<std::string, std::vector<std::string>> domain;

public:
    // Constructor: takes a reference to the Bayesian network, the dataset, and the list of attribute names.
    Cleaner(BayesianNetwork& bn, const DataFrameCleaner& data, const std::vector<std::string>& attributes);

    // Set the candidate domain for a given attribute.
    void setDomain(const std::string& attribute, const std::vector<std::string>& values);

    // Clean the dataset by applying a simplified Bayesian repair logic.
    DataFrameCleaner cleanData();
};

#endif // CLEANER_H