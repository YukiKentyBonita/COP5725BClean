#ifndef COMPENSATIVEPARAMETER_H
#define COMPENSATIVEPARAMETER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include "../dataset.h" // Use your existing AttrInfo and DataFrame definitions
// #include <Eigen/Dense>

using std::string;
using std::vector;
using std::unordered_map;
using std::map;

using Row = unordered_map<string, string>;
using Data = vector<Row>;

class BayesianModel {
public:
    virtual vector<string> get_parents(const string& node) const = 0;
    virtual vector<string> get_children(const string& node) const = 0;
};

class CompensativeParameter {
public:
    CompensativeParameter(
        const map<string, AttrInfo>& attr_type,
        const map<string, vector<string>>& domain,
        const unordered_map<string,
            unordered_map<string,
                unordered_map<string,
                    unordered_map<string, double>>>>& occurrence,
        std::shared_ptr<BayesianModel> model,
        const Data& df
    );

    map<string, double> return_penalty(
        const string& obs,
        const string& attr,
        int index,
        const Row& data_line,
        const vector<string>& prior
    );

private:
    map<string, AttrInfo> attr_type;
    map<string, vector<string>> domain;
    unordered_map<string,
        unordered_map<string,
            unordered_map<string,
                unordered_map<string, double>>>> occurrence;
    std::shared_ptr<BayesianModel> model;
    Data df;

    bool isValid(const string& attr, const string& val);
    int levenshtein(const string& s1, const string& s2);
};

#endif // COMPENSATIVEPARAMETER_H