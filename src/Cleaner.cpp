#include "../include/Cleaner.h"
#include <cmath>
#include <iostream>
#include <algorithm>

// Constructor: initializes the Cleaner with the provided Bayesian network, dataset, and attributes.
Cleaner::Cleaner(BayesianNetwork& bn, const DataFrameCleaner& data, const std::vector<std::string>& attributes)
    : bn(bn), data(data), attributes(attributes)
{
    // Initialize the domain for each attribute as empty.
    for (const auto& attr : attributes) {
        domain[attr] = std::vector<std::string>();
    }
}

// Sets the candidate domain for an attribute.
void Cleaner::setDomain(const std::string& attribute, const std::vector<std::string>& values) {
    domain[attribute] = values;
}

// The cleanData method applies a simple repair logic:
// For each cell, it checks the candidate domain and picks the candidate with the highest log-probability
// (as obtained from the Bayesian network). If the current value is the best, it remains unchanged.
DataFrameCleaner Cleaner::cleanData() {
    int n = data.size();
    int m = attributes.size();
    DataFrameCleaner cleanedData = data;  // Start with the original data

    // Iterate over each row and column.
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            std::string attr = attributes[j];
            std::string currentValue = data[i][j];
            // Get the probability of the current value from the Bayesian network.
            double bestScore = std::log(bn.getProbability(attr, currentValue));
            std::string bestCandidate = currentValue;

            // For each candidate in the domain, compute its log probability.
            for (const std::string& candidate : domain[attr]) {
                double score = std::log(bn.getProbability(attr, candidate));
                if (score > bestScore) {
                    bestScore = score;
                    bestCandidate = candidate;
                }
            }
            // Replace the cell value with the candidate that has the highest score.
            cleanedData[i][j] = bestCandidate;
        }
    }
    return cleanedData;
}