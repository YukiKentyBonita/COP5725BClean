#include "../include/BNStructure.h"
#include <iostream>
#include <vector>

int main()
{
    // Dummy dataset: 3 attributes, 4 samples
    std::vector<std::vector<std::string>> data = {
        {"sunny", "hot", "high"},
        {"sunny", "hot", "high"},
        {"overcast", "hot", "high"},
        {"rainy", "mild", "high"},
        {"rainy", "cool", "normal"},
        {"rainy", "cool", "normal"},
        {"overcast", "cool", "normal"},
        {"sunny", "mild", "high"},
        {"sunny", "cool", "normal"},
        {"rainy", "mild", "normal"},
        {"sunny", "mild", "normal"},
        {"overcast", "mild", "high"},
        {"overcast", "hot", "normal"},
        {"rainy", "mild", "high"}};

    // Fix edges for testing (optional, for "fix" mode)
    std::vector<Edge> fixed_edges = {
        Edge("Attr0", "Attr1"),
        Edge("Attr1", "Attr2")};

    // === Test 1: using fixed edges ===
    std::cout << "==== Test FIXED structure ====" << std::endl;
    BNStructure bn_fixed(data, "", "fix", fixed_edges);
    BNResult result_fixed = bn_fixed.get_bn();

    // === Test 2: using approximate structure learning ===
    std::cout << "\n==== Test APPROXIMATE structure ====" << std::endl;
    BNStructure bn_appr(data, "", "appr", {});
    BNResult result_appr = bn_appr.get_bn();

    std::cout << "Confirmed edges from approximate structure:" << std::endl;
    for (const auto &pair : result_appr.full_graph.adjacency_list)
    {
        for (const auto &child : pair.second)
        {
            std::cout << "(" << pair.first << ", " << child << ")" << std::endl;
        }
    }

    return 0;
}