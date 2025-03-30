#ifndef CLEANER_H
#define CLEANER_H

#include "BayesianNetwork.h"
#include <vector>
#include <string>
#include <map>

// Define a simple alias for our dataset.
// A row is a vector of strings (each string represents a cell).
using Row = std::vector<std::string>;
// The dataset is a vector of rows.
using DataFrame = std::vector<Row>;

// Cleaner class encapsulates the data cleaning (repair) process.
class Cleaner {
private:
    // Reference to the Bayesian network (assumed to be already constructed)
    BayesianNetwork& bn;
    // The dataset to be cleaned.
    DataFrame data;
    // List of attribute names corresponding to each column.
    std::vector<std::string> attributes;
    // Domain of possible values for each attribute.
    // For example: domain["state"] might be {"NY", "CA", "TX", ...}
    std::map<std::string, std::vector<std::string>> domain;

public:
    // Constructor: takes a reference to the Bayesian network, the dataset,
    // and the list of attribute names.
    Cleaner(BayesianNetwork& bn, const DataFrame& data, const std::vector<std::string>& attributes);

    // Set the domain for a given attribute.
    void setDomain(const std::string& attribute, const std::vector<std::string>& values);

    // Clean the dataset by applying Bayesian inference to each cell.
    DataFrame cleanData();
};

#endif // CLEANER_H