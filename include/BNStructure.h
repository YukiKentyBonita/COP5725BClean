#ifndef BNStructure_H
#define BNStructure_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>

// Directed edge
struct Edge
{
    std::string from;
    std::string to;

    Edge(const std::string &f, const std::string &t) : from(f), to(t) {}

    bool operator<(const Edge &other) const
    {
        return (from < other.from) || (from == other.from && to < other.to);
    }
};

// Bayesian Network graph structure
struct BNGraph
{
    std::map<std::string, std::set<std::string>> adjacency_list;
};

// Result of get_bn()
struct BNResult
{
    BNGraph full_graph;
    std::unordered_map<std::string, BNGraph> partition_graphs;
};

class BNStructure
{
public:
    BNStructure(const std::vector<std::vector<std::string>> &data,
                const std::string &model_path,
                const std::string &model_choice,
                const std::vector<Edge> &fix_edge,
                const std::string &model_save_path = "");

    BNResult get_bn();

private:
    std::vector<std::vector<std::string>> data;
    std::string model_path;
    std::string model_choice;
    std::string model_save_path;
    std::vector<Edge> fix_edge;

    BNGraph model;
    std::unordered_map<std::string, BNGraph> model_dict;

    std::vector<Edge> get_rel(const std::vector<std::vector<std::string>> &data);
};

#endif // BNStructure_H
