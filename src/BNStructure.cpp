#include "../include/BNStructure.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <set>
#include "BNStructure.h"

using namespace std;

BNStructure::BNStructure(const DataFrame &data,
                         const string &model_path,
                         const string &model_choice,
                         const std::vector<Edge> &fix_edge,
                         const string &model_save_path)
    : data(data), model_path(model_path), model_choice(model_choice), fix_edge(fix_edge), model_save_path(model_save_path) {}

void BNStructure::print_bn_result(const BNResult &result)
{
    std::cout << "=== BNResult ===" << std::endl;

    std::cout << "Full Graph:" << std::endl;
    print_graph(result.full_graph); // 假设你有 print_graph 函数

    std::cout << "Partition Graphs:" << std::endl;
    for (const auto &pair : result.partition_graphs)
    {
        std::cout << "Partition: " << pair.first << std::endl;
        print_graph(pair.second); // 同样假设你有 print_graph 函数
    }
}

void BNStructure::print_graph(const BNGraph &graph)
{
    for (const auto &node : graph.adjacency_list)
    {
        std::cout << "Node: " << node.first << " -> ";
        for (const auto &neighbor : node.second)
        {
            std::cout << neighbor << " ";
        }
        std::cout << std::endl;
    }
}

BNResult BNStructure::get_bn()
{
    vector<string> attributes;
    if (!data.rows.empty())
    {
        for (size_t i = 0; i < data.rows[0].size(); ++i)
            attributes.push_back("Attr" + to_string(i));
    }

    for (const auto &attr : attributes)
    {
        std::cout << attr << std::endl;
    }

    BNGraph G;
    unordered_map<string, BNGraph> model_dict;

    if (!model_path.empty())
    {
        cout << "Model loading from file is not implemented in this C++ version." << endl;
    }
    else
    {
        if (model_choice == "appr")
        {
            auto start = chrono::high_resolution_clock::now();
            vector<Edge> Edges = get_rel(data);

            for (const auto &attr : attributes)
                G.adjacency_list[attr] = set<string>();
            for (const auto &edge : Edges)
                G.adjacency_list[edge.from].insert(edge.to);

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> diff = end - start;
            cout << "Approximate structure time used: " << diff.count() << " seconds" << endl;
        }
        else if (model_choice == "fix")
        {
            for (const auto &attr : attributes)
                G.adjacency_list[attr] = set<string>();
            for (const auto &edge : fix_edge)
                G.adjacency_list[edge.from].insert(edge.to);
        }
        else
        {
            cout << "Only 'appr' and 'fix' modes are implemented in this C++ version." << endl;
        }
    }

    for (const auto &node : G.adjacency_list)
    {
        const string &key = node.first;
        set<string> parents;
        set<string> children = node.second;

        for (const auto &potential_parent : G.adjacency_list)
        {
            if (potential_parent.second.count(key))
                parents.insert(potential_parent.first);
        }

        set<string> relevant_nodes = parents;
        relevant_nodes.insert(children.begin(), children.end());
        relevant_nodes.insert(key);

        BNGraph temp_graph;
        for (const string &n : relevant_nodes)
            temp_graph.adjacency_list[n] = set<string>();
        for (const string &p : parents)
            temp_graph.adjacency_list[p].insert(key);
        for (const string &c : children)
            temp_graph.adjacency_list[key].insert(c);

        model_dict[key] = temp_graph;
    }

    cout << "Nodes: ";
    for (const auto &p : G.adjacency_list)
        cout << p.first << " ";
    cout << endl
         << "Edges: ";
    for (const auto &p : G.adjacency_list)
        for (const auto &c : p.second)
            cout << "(" << p.first << ", " << c << ") ";
    cout << endl;

    model = G;
    this->model_dict = model_dict;

    BNResult result;
    result.full_graph = G;
    result.partition_graphs = model_dict;
    return result;
}

vector<Edge> BNStructure::get_rel(const DataFrame &data)
{
    vector<string> attrs;
    if (!data.rows.empty())
    {
        for (size_t i = 0; i < data.rows[0].size(); ++i)
            attrs.push_back("Attr" + to_string(i));
    }

    int n = data.rows.size();
    int m = attrs.size();
    int max_indegree = 2;

    // 1. 计算所有互信息
    map<pair<string, string>, double> mi_map;

    for (int i = 0; i < m; ++i)
    {
        for (int j = 0; j < m; ++j)
        {
            if (i == j)
                continue;

            map<string, int> count_i, count_j;
            map<pair<string, string>, int> count_ij;

            for (int k = 0; k < n; ++k)
            {
                string vi = data.rows[k][i];
                string vj = data.rows[k][j];
                count_i[vi]++;
                count_j[vj]++;
                count_ij[{vi, vj}]++;
            }

            double mi = 0.0;
            for (const auto &p : count_ij)
            {
                string vi = p.first.first;
                string vj = p.first.second;
                double p_ij = (double)p.second / n;
                double p_i = (double)count_i[vi] / n;
                double p_j = (double)count_j[vj] / n;
                mi += p_ij * log((p_ij / (p_i * p_j)) + 1e-9);
            }

            mi_map[{attrs[i], attrs[j]}] = mi;
        }
    }

    // 2. 每个节点选择互信息最高的 max_indegree 个父节点
    unordered_map<string, vector<pair<string, double>>> candidate_parents;

    for (const auto &entry : mi_map)
    {
        string from = entry.first.first;
        string to = entry.first.second;
        double mi = entry.second;

        if (mi > 0.01)
        {
            candidate_parents[to].emplace_back(from, mi);
        }
    }

    vector<Edge> final_edges;
    for (auto &[to, candidates] : candidate_parents)
    {
        sort(candidates.begin(), candidates.end(),
             [](const pair<string, double> &a, const pair<string, double> &b)
             {
                 return a.second > b.second;
             });

        set<string> existing_parents;
        for (int i = 0; i < (int)candidates.size() && (int)existing_parents.size() < max_indegree; ++i)
        {
            string parent = candidates[i].first;

            // 防止双向边
            bool conflict = false;
            for (const auto &e : final_edges)
            {
                if (e.from == to && e.to == parent)
                {
                    conflict = true;
                    break;
                }
            }

            if (!conflict)
            {
                final_edges.emplace_back(parent, to);
                existing_parents.insert(parent);
            }
        }
    }

    cout << "Discovered edges: [";
    for (const auto &e : final_edges)
        cout << "(" << e.from << ", " << e.to << ") ";
    cout << "]" << endl;

    return final_edges;
}