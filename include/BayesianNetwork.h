#ifndef BAYESIANNETWORK_H
#define BAYESIANNETWORK_H

#include <unordered_map>
#include <string>

class BayesianNetwork {
public:
    // The probabilityTable stores, for each attribute, a mapping from candidate values to their probabilities.
    std::unordered_map<std::string, std::unordered_map<std::string, double>> probabilityTable;

    // Adds or updates the probability for a given attribute-value pair.
    void addProbability(const std::string& attribute, const std::string& value, double probability);

    // Retrieves the probability for a given attribute-value pair.
    // If the pair isn't found, returns a small default value to avoid zero probabilities.
    double getProbability(const std::string& attribute, const std::string& value);
};

#endif // BAYESIANNETWORK_H