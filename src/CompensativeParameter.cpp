#include "../include/CompensativeParameter.h"
#include "../include/StaticBayesianModel.h"
#include <regex>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

CompensativeParameter::CompensativeParameter(
    const map<string, AttrInfo>& attr_type,
    const map<string, vector<string>>& domain,
    const decltype(occurrence)& occurrence,
    std::shared_ptr<BayesianModel> model,
    const Data& df)
    : attr_type(attr_type), domain(domain), occurrence(occurrence), model(model), df(df) {}

bool CompensativeParameter::isValid(const string& attr, const string& val) {
    const auto& info = attr_type.at(attr);
    if (info.allowNull == "N" && val == "A Null Cell") return false;
    if (!info.pattern.empty()) {
        try {
            std::regex regex_pattern(info.pattern);
            return std::regex_match(val, regex_pattern);
        } catch (...) {
            return false;
        }
    }
    return true;
}

int CompensativeParameter::levenshtein(const string& s1, const string& s2) {
    const size_t m = s1.size(), n = s2.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));
    for (size_t i = 0; i <= m; ++i) dp[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= n; ++j) dp[0][j] = static_cast<int>(j);
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = std::min({ dp[i - 1][j - 1], dp[i][j - 1], dp[i - 1][j] }) + 1;
        }
    }
    return dp[m][n];
}

map<string, double> CompensativeParameter::return_penalty(
    const string& obs,
    const string& attr,
    int index,
    const Row& data_line,
    const vector<string>& prior
) {
    map<string, double> p_obs_G_cand;
    map<string, double> domain_base, cooccurance_dist;
    double total_ground = 0.0, total_cooccur = 0.0;

    string obs_clean = (obs == "A Null Cell" && attr_type[attr].allowNull == "N") ? "" : obs;

    // Precompute once
    auto parents = model->get_parents(attr);
    auto children = model->get_children(attr);

    std::cout << "Attributes in data_line:" << std::endl;
    for (const auto& [key, val] : data_line) {
        std::cout << "  " << key << ": " << val << std::endl;
    }

    for (const string& groud : prior) {
        domain_base[groud] = static_cast<double>(1 + levenshtein(obs_clean, groud));
        total_ground += domain_base[groud];

        vector<double> co_vec;
        for (const auto& [other_attr, other_val] : data_line) {
            if (other_attr == attr) continue;
            if (std::find(parents.begin(), parents.end(), other_attr) != parents.end()) continue;
            //if (std::find(children.begin(), children.end(), other_attr) != children.end()) continue;

            double score = 0.0;
            try {
                score = occurrence.at(attr).at(groud).at(other_attr).at(other_val);
            } catch (...) {
                score = 0.0;
            }
            std::cout << "  Checking attr: " << other_attr << ", val: " << other_val
          << " → score = " << score << "\n";

            co_vec.push_back(score);
        }

        std::cout << "Prior: " << groud << " co_vec: [";
        for (double v : co_vec) std::cout << v << " ";
        std::cout << "]" << std::endl;

        double l2 = std::inner_product(co_vec.begin(), co_vec.end(), co_vec.begin(), 0.0);
        cooccurance_dist[groud] = std::sqrt(l2) + 1.0;
        total_cooccur += cooccurance_dist[groud];
    }

    for (const string& groud : prior) {
        if (!isValid(attr, groud)) {
            domain_base[groud] = 0.0;
            cooccurance_dist[groud] = 0.0;
            p_obs_G_cand[groud] = 0.0;
        } else {
            double norm_score = (total_cooccur > 0.0) ? (cooccurance_dist[groud] / total_cooccur) : 0.0;
            p_obs_G_cand[groud] = norm_score;
        }
    }

    return p_obs_G_cand;
}