#ifndef STATIC_BAYESIAN_MODEL_H
#define STATIC_BAYESIAN_MODEL_H

#include "CompensativeParameter.h"  // for BayesianModel

class StaticBayesianModel : public BayesianModel {
private:
    std::unordered_map<std::string, std::vector<std::string>> parent_map;
    std::unordered_map<std::string, std::vector<std::string>> child_map;

public:
    StaticBayesianModel() { /*TEMP CODE: UPDATED AFTER BN AVAILABLE*/
        // Define simple static structure: Name → City → Country
        add_edge("Name", "City");
        add_edge("City", "Country");
    }

    void add_edge(const std::string& parent, const std::string& child) {
        parent_map[child].push_back(parent);
        child_map[parent].push_back(child);
    }

    std::vector<std::string> get_parents(const std::string& node) const override {
        auto it = parent_map.find(node);
        return (it != parent_map.end()) ? it->second : std::vector<std::string>{};
    }

    std::vector<std::string> get_children(const std::string& node) const override {
        auto it = child_map.find(node);
        return (it != child_map.end()) ? it->second : std::vector<std::string>{};
    }
};

#endif // STATIC_BAYESIAN_MODEL_H