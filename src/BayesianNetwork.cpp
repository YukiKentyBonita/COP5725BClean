#include "../include/BayesianNetwork.h"

// Adds or updates the probability for the specified attribute and value.
void BayesianNetwork::addProbability(const std::string& attribute, const std::string& value, double probability) {
    probabilityTable[attribute][value] = probability;
}

// Retrieves the probability for the specified attribute and value.
// Returns a small default probability (1e-9) if the attribute or value is not found.
double BayesianNetwork::getProbability(const std::string& attribute, const std::string& value) {
    if (probabilityTable.find(attribute) != probabilityTable.end() &&
        probabilityTable[attribute].find(value) != probabilityTable[attribute].end()) {
        return probabilityTable[attribute][value];
    }
    return 1e-9; // Return a very small probability for unseen values
}