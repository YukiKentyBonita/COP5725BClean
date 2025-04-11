#include "CompensativeParameter.h"
#include <cmath>
#include <regex>
#include <algorithm>
#include <vector>
#include <numeric>
#include <iostream>

// ----------------- Helper Functions -----------------

// A simple implementation of the Levenshtein distance.
int CompensativeParameter::levenshtein_distance(const string& s1, const string& s2) {
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));
    for (size_t i = 0; i <= len1; i++) d[i][0] = i;
    for (size_t j = 0; j <= len2; j++) d[0][j] = j;
    for (size_t i = 1; i <= len1; i++) {
        for (size_t j = 1; j <= len2; j++) {
            d[i][j] = std::min({ d[i - 1][j] + 1,
                                 d[i][j - 1] + 1,
                                 d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1) });
        }
    }
    return d[len1][len2];
}

// Compute the Euclidean norm of a vector of doubles.
double CompensativeParameter::euclidean_norm(const vector<double>& vec) {
    double sum_sq = 0.0;
    for (double v : vec) {
        sum_sq += v * v;
    }
    return std::sqrt(sum_sq);
}

// ----------------- Constructor -----------------

CompensativeParameter::CompensativeParameter(const map<string, AttrInfo>& attr_type,
                                             const unordered_map<string, unordered_map<string, int>>& domain,
                                             const unordered_map<string,
                                                unordered_map<string,
                                                    unordered_map<string,
                                                        unordered_map<string, double>>>>& occurrence,
                                             const BNGraph& model,
                                             const DataFrame& df)
    : attr_type(attr_type), domain(domain), occurrence(occurrence), model(model), df(df)
{
    // tf_idf is initially empty.
}

// ----------------- return_penalty -----------------

unordered_map<string, double> CompensativeParameter::return_penalty(const string& obs,
                                                                    const string& attr,
                                                                    int index,
                                                                    const Row& data_line,
                                                                    const vector<string>& prior) {
    unordered_map<string, double> p_obs_G_cand;
    unordered_map<string, double> domain_base;
    unordered_map<string, double> cooccurance_dist;
    double total_ground = 0;
    double total_cooccur = 0;

    // Retrieve the occurrence data for attr.
    auto occ_it = occurrence.find(attr);
    if (occ_it == occurrence.end()) {
        return p_obs_G_cand;
    }
    const auto& occ_attr = occ_it->second;

    // If the observation equals "A Null Cell" and the attribute disallows nulls, set it to an empty string.
    string local_obs = obs;
    if (obs == "A Null Cell" && attr_type.at(attr).allowNull == "N") {
        local_obs = "";
    }

    // For each candidate in prior, compute an edit distance based base and accumulate totals.
    for (const auto& candidate : prior) {
        int dist = levenshtein_distance(local_obs, candidate);
        domain_base[candidate] = 1 + dist;
        total_ground += domain_base[candidate];

        // Build the co-occurrence vector.
        vector<double> cooccur_vec;
        // For each other attribute in attr_type:
        for (const auto& pair : attr_type) {
            string other_attr = pair.first;
            if (other_attr == attr) continue;

            // Define local lambdas for getting parents and children from the BN model.
            auto get_parents = [this](const string& node) -> vector<string> {
                vector<string> parents;
                for (const auto& p : model.adjacency_list) {
                    if (p.second.find(node) != p.second.end())
                        parents.push_back(p.first);
                }
                return parents;
            };
            auto get_children = [this](const string& node) -> vector<string> {
                vector<string> children;
                auto it = model.adjacency_list.find(node);
                if (it != model.adjacency_list.end()) {
                    for (const auto& child : it->second)
                        children.push_back(child);
                }
                return children;
            };

            // Skip other_attr if it is a parent or child of attr.
            vector<string> parents_of_attr = get_parents(attr);
            vector<string> children_of_attr = get_children(attr);
            bool skip = false;
            for (const auto &p : parents_of_attr)
                if (p == other_attr) { skip = true; break; }
            for (const auto &c : children_of_attr)
                if (c == other_attr) { skip = true; break; }
            if (skip) continue;

            // Get other_val from the current data row.
            string other_val = "";
            auto it = data_line.find(other_attr);
            if (it != data_line.end())
                other_val = it->second;

            // Look up the co-occurrence score for candidate, other_attr, other_val.
            double score = 0;
            auto occ_candidate_it = occ_attr.find(candidate);
            if (occ_candidate_it != occ_attr.end()) {
                const auto& other_attr_map = occ_candidate_it->second;
                auto occ_other_it = other_attr_map.find(other_attr);
                if (occ_other_it != other_attr_map.end()) {
                    const auto& val_map = occ_other_it->second;
                    auto score_it = val_map.find(other_val);
                    if (score_it != val_map.end())
                        score = score_it->second;
                }
            }
            cooccur_vec.push_back(score);
        }
        double norm_val = euclidean_norm(cooccur_vec) + 1;
        cooccurance_dist[candidate] = norm_val;
        total_cooccur += norm_val;
    }

    // Combine the base computations with validity checks.
    for (const auto& candidate : prior) {
        // Check if candidate is valid: it must pass the AllowNull and pattern requirements.
        bool valid = false;
        const auto& info = attr_type.at(attr);
        string allowNull = info.allowNull;
        string pattern = info.pattern;  // assume empty string means no pattern
        if ( (allowNull == "Y" || (allowNull == "N" && candidate != "A Null Cell")) &&
             (pattern.empty() || (!pattern.empty() && std::regex_search(candidate, std::regex(pattern))) ) )
        {
            valid = true;
        }
        if (!valid) {
            domain_base[candidate] = 0;
            cooccurance_dist[candidate] = 0;
            p_obs_G_cand[candidate] = 0;
        } else {
            double norm_domain = (total_ground != 0) ? domain_base[candidate] / total_ground : 0;
            double norm_cooccur = (total_cooccur != 0) ? cooccurance_dist[candidate] / total_cooccur : 0;
            p_obs_G_cand[candidate] = norm_cooccur;
        }
    }
    return p_obs_G_cand;
}

// ----------------- return_penalty_test -----------------

unordered_map<string, double> CompensativeParameter::return_penalty_test(const string& obs,
                                                                           const string& attr,
                                                                           int index,
                                                                           const Row& data_line,
                                                                           const vector<string>& prior,
                                                                           const vector<string>& attr_order) {
    unordered_map<string, double> p_obs_G_cand;
    // If no TF-IDF data has been computed for attr, return default scores (all ones).
    if (tf_idf.find(attr) == tf_idf.end() || tf_idf[attr] == nullptr) {
        for (const auto& candidate : prior)
            p_obs_G_cand[candidate] = 1;
        return p_obs_G_cand;
    }
    auto tfidfData = tf_idf[attr];
    // Build a combined context string using the attributes specified.
    string obs_combine = "";
    for (const auto& at : tfidfData->combine_attrs) {
        auto it = data_line.find(at);
        if (it != data_line.end())
            obs_combine += "," + it->second;
    }
    // For each candidate ground value, calculate a TF-IDF based score.
    for (const auto& ground : prior) {
        string obs_context = ground + obs_combine;
        int tf = 0;
        auto it_tf = tfidfData->dic.find(obs_context);
        if (it_tf != tfidfData->dic.end())
            tf = it_tf->second;
        if (tf == 0)
            continue;
        int dic_idf_value = 0;
        auto it_idf = tfidfData->dic_idf.find(obs);
        if (it_idf != tfidfData->dic_idf.end())
            dic_idf_value = it_idf->second;
        double idf = std::log((double)df.rows.size() / (dic_idf_value + 1));
        if (idf == 0)
            continue;
        p_obs_G_cand[ground] = tf * idf;
    }
    return p_obs_G_cand;
}

// ----------------- Helper for value counts -----------------

unordered_map<string, int> value_counts(const DataFrame& df, const string& col_name) {
    unordered_map<string, int> counts;

    // Step 1: Find the index of the column
    auto it = std::find(df.columns.begin(), df.columns.end(), col_name);
    if (it == df.columns.end()) {
        std::cerr << "Column \"" << col_name << "\" not found in DataFrame.\n";
        return counts;
    }

    int col_index = std::distance(df.columns.begin(), it);

    // Step 2: Count values from that column in each row
    for (const auto& row : df.rows) {
        if (col_index < row.size()) {
            counts[row[col_index]]++;
        }
    }

    return counts;
}

// ----------------- init_tf_idf -----------------

void CompensativeParameter::init_tf_idf(const vector<string>& attr_order) {
    for (const auto& pair : attr_type) {
        string attr = pair.first;
        vector<string> combine_attrs;
        // Define local lambdas to retrieve parents and children from the BN model.
        auto get_parents = [this](const string& node) -> vector<string> {
            vector<string> parents;
            for (const auto& p : model.adjacency_list) {
                if (p.second.find(node) != p.second.end())
                    parents.push_back(p.first);
            }
            return parents;
        };
        auto get_children = [this](const string& node) -> vector<string> {
            vector<string> children;
            auto it = model.adjacency_list.find(node);
            if (it != model.adjacency_list.end()) {
                for (const auto& child : it->second)
                    children.push_back(child);
            }
            return children;
        };

        // For each attribute in attr_order, check if it is related (as parent or child).
        for (const auto& at : attr_order) {
            if (at == attr)
                continue;
            vector<string> parents = get_parents(attr);
            vector<string> children = get_children(attr);
            bool related = false;
            for (const auto& p : parents)
                if (p == at) { related = true; break; }
            for (const auto& c : children)
                if (c == at) { related = true; break; }
            if (related)
                combine_attrs.push_back(at);
        }
        // If no related attributes, mark this attribute’s TF-IDF data as null.
        if (combine_attrs.empty()) {
            tf_idf[attr] = nullptr;
            continue;
        }
        // Build a new column name by joining combine_attrs with underscores and appending "_TempAttribute".
        string context_attr;
        for (size_t i = 0; i < combine_attrs.size(); i++) {
            context_attr += combine_attrs[i];
            if (i != combine_attrs.size() - 1)
                context_attr += "_";
        }
        context_attr += "_TempAttribute";

        // Make a copy of the DataFrame.
        DataFrame df_copy = df;

        // Add context_attr to columns if it doesn't already exist.
        auto it_col = std::find(df_copy.columns.begin(), df_copy.columns.end(), context_attr);
        if (it_col == df_copy.columns.end()) {
            df_copy.columns.push_back(context_attr);
        }
        int context_col_idx = std::distance(df_copy.columns.begin(),
                                            std::find(df_copy.columns.begin(), df_copy.columns.end(), context_attr));

        // Ensure each row is resized to accommodate the new column
        for (auto& row : df_copy.rows) {
            if (row.size() < df_copy.columns.size()) {
                row.resize(df_copy.columns.size(), ""); // Fill new entries with empty string
            }
        }

        // Update the context column by concatenating values of combine_attrs
        for (const auto& at : combine_attrs) {
            // Find the column index for 'at'
            auto at_it = std::find(df_copy.columns.begin(), df_copy.columns.end(), at);
            if (at_it == df_copy.columns.end()) continue; // skip if not found
            int at_idx = std::distance(df_copy.columns.begin(), at_it);

            for (auto& row : df_copy.rows) {
                if (at_idx < row.size()) {
                    if (row[context_col_idx].empty()) {
                        row[context_col_idx] = row[at_idx];
                    } else {
                        row[context_col_idx] += "," + row[at_idx];
                    }
                }
            }
        }

        // Create new column name
        string context_obs = attr + "_" + context_attr;

        // Add to columns if not already there
        auto it_obs = std::find(df_copy.columns.begin(), df_copy.columns.end(), context_obs);
        if (it_obs == df_copy.columns.end()) {
            df_copy.columns.push_back(context_obs);
        }
        int context_obs_idx = std::distance(df_copy.columns.begin(),
                                            std::find(df_copy.columns.begin(), df_copy.columns.end(), context_obs));

        // Get column indices for attr and context_attr
        auto it_attr = std::find(df_copy.columns.begin(), df_copy.columns.end(), attr);
        auto it_context = std::find(df_copy.columns.begin(), df_copy.columns.end(), context_attr);

        if (it_attr == df_copy.columns.end() || it_context == df_copy.columns.end()) {
            std::cerr << "Attribute or context attribute not found in columns.\n";
            // You could throw an exception here if needed
            return;
        }

        int attr_idx = std::distance(df_copy.columns.begin(), it_attr);
        int context_idx = std::distance(df_copy.columns.begin(), it_context);

        // Resize rows and assign values
        for (auto& row : df_copy.rows) {
            if (row.size() < df_copy.columns.size()) {
                row.resize(df_copy.columns.size(), "");
            }

            string val_attr = (attr_idx < row.size()) ? row[attr_idx] : "";
            string val_context = (context_idx < row.size()) ? row[context_idx] : "";
            row[context_obs_idx] = val_attr + "," + val_context;
        }
        // Compute value counts (term frequencies) for context_obs and for attr.
        unordered_map<string, int> dic = value_counts(df_copy, context_obs);
        unordered_map<string, int> dic_idf = value_counts(df_copy, attr);
        // Store the computed TF-IDF data.
        std::shared_ptr<TFIDFData> tfdata = std::make_shared<TFIDFData>();
        tfdata->combine_attrs = combine_attrs;
        tfdata->dic = dic;
        tfdata->dic_idf = dic_idf;
        tf_idf[attr] = tfdata;
    }
}