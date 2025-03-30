#include "../include/Cleaner.h"
#include <cmath>
#include <iostream>
#include <algorithm>

// Constructor: initialize with the Bayesian network, dataset, and attributes.
Cleaner::Cleaner(BayesianNetwork& bn, const DataFrame& data, const std::vector<std::string>& attributes)
    : bn(bn), data(data), attributes(attributes)
{
    // Optionally, initialize the domain for each attribute to an empty vector.
    for (const auto& attr : attributes) {
        domain[attr] = std::vector<std::string>();
    }
}

// Set the domain (list of candidate values) for an attribute.
void Cleaner::setDomain(const std::string& attribute, const std::vector<std::string>& values) {
    domain[attribute] = values;
}

// The cleanData method iterates over each cell, computes a probability for each candidate,
// and selects the candidate with the highest score.
// For simplicity, the scoring is computed as the logarithm of the probability from the BN.
DataFrame Cleaner::cleanData() {
    int n = data.size();
    int m = attributes.size();
    DataFrame cleanedData = data;  // Start with the original data

    // Iterate over every row and column.
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            std::string attr = attributes[j];
            std::string currentValue = data[i][j];
            // Get the probability of the current value from the Bayesian network.
            double bestScore = std::log(bn.getProbability(attr, currentValue));
            std::string bestCandidate = currentValue;

            // Look up candidate values from the domain.
            // In a full implementation, you might generate these candidates dynamically.
            for (const std::string& candidate : domain[attr]) {
                double score = std::log(bn.getProbability(attr, candidate));
                if (score > bestScore) {
                    bestScore = score;
                    bestCandidate = candidate;
                }
            }
            // Replace the cell with the candidate that has the highest probability.
            cleanedData[i][j] = bestCandidate;
        }
    }
    return cleanedData;
}