// #include "CompensativeParameter.h"
// #include <cmath>
// #include <regex>
// #include <algorithm>
// #include <vector>
// #include <numeric>
// #include <iostream>

// // ----------------- Helper Functions -----------------

// static std::string canonical(const std::string &s) {
//     std::string out;
//     for (char ch : s)
//         if (!std::isspace(ch) && ch!='%')          // drop blanks + '%'
//             out.push_back(std::tolower(ch));       // lower‑case
//     return out;
// }

// // A simple implementation of the Levenshtein distance.
// int CompensativeParameter::levenshtein_distance(const string& s1, const string& s2) {
//     const size_t len1 = s1.size(), len2 = s2.size();
//     std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));
//     for (size_t i = 0; i <= len1; i++) d[i][0] = i;
//     for (size_t j = 0; j <= len2; j++) d[0][j] = j;
//     for (size_t i = 1; i <= len1; i++) {
//         for (size_t j = 1; j <= len2; j++) {
//             d[i][j] = std::min({ d[i - 1][j] + 1,
//                                  d[i][j - 1] + 1,
//                                  d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1) });
//         }
//     }
//     return d[len1][len2];
// }

// // Compute the Euclidean norm of a vector of doubles.
// double CompensativeParameter::euclidean_norm(const vector<double>& vec) {
//     double sum_sq = 0.0;
//     for (double v : vec) {
//         sum_sq += v * v;
//     }
//     return std::sqrt(sum_sq);
// }

// // ----------------- Constructor -----------------

// CompensativeParameter::CompensativeParameter(const map<string, AttrInfo>& attr_type,
//                                              const unordered_map<string, unordered_map<string, int>>& domain,
//                                              const unordered_map<string,
//                                                 unordered_map<string,
//                                                     unordered_map<string,
//                                                         unordered_map<string, double>>>>& occurrence,
//                                              const BNGraph& model,
//                                              const DataFrame& df)
//     : attr_type(attr_type), domain(domain), occurrence(occurrence), model(model), df(df)
// {
//     // tf_idf is initially empty.
// }

// // ----------------- return_penalty -----------------

// unordered_map<string, double> CompensativeParameter::return_penalty(const string& obs,
//                                                                     const string& attr,
//                                                                     int index,
//                                                                     const Row& data_line,
//                                                                     const vector<string>& prior) {
//     unordered_map<string, double> p_obs_G_cand;
//     unordered_map<string, double> domain_base;
//     unordered_map<string, double> cooccurance_dist;
//     double total_ground = 0;
//     double total_cooccur = 0;

//     // Retrieve the occurrence data for attr.
//     auto occ_it = occurrence.find(attr);
//     if (occ_it == occurrence.end()) {
//         return p_obs_G_cand;
//     }
//     const auto& occ_attr = occ_it->second;

//     // If the observation equals "A Null Cell" and the attribute disallows nulls, set it to an empty string.
//     string local_obs = obs;
//     if (obs == "A Null Cell" && attr_type.at(attr).allowNull == "N") {
//         local_obs = "";
//     }

//     // For each candidate in prior, compute an edit distance based base and accumulate totals.
//     for (const auto& candidate : prior) {
//         int dist = levenshtein_distance(canonical(local_obs), canonical(candidate));
//         domain_base[candidate] = 1 + dist;
//         total_ground += domain_base[candidate];

//         // Build the co-occurrence vector.
//         vector<double> cooccur_vec;
//         // For each other attribute in attr_type:
//         for (const auto& pair : attr_type) {
//             string other_attr = pair.first;
//             if (other_attr == attr) continue;

//             // Define local lambdas for getting parents and children from the BN model.
//             auto get_parents = [this](const string& node) -> vector<string> {
//                 vector<string> parents;
//                 for (const auto& p : model.adjacency_list) {
//                     if (p.second.find(node) != p.second.end())
//                         parents.push_back(p.first);
//                 }
//                 return parents;
//             };
//             auto get_children = [this](const string& node) -> vector<string> {
//                 vector<string> children;
//                 auto it = model.adjacency_list.find(node);
//                 if (it != model.adjacency_list.end()) {
//                     for (const auto& child : it->second)
//                         children.push_back(child);
//                 }
//                 return children;
//             };

//             // Skip other_attr if it is a parent or child of attr.
//             vector<string> parents_of_attr = get_parents(attr);
//             vector<string> children_of_attr = get_children(attr);
//             bool skip = false;
//             for (const auto &p : parents_of_attr)
//                 if (p == other_attr) { skip = true; break; }
//             for (const auto &c : children_of_attr)
//                 if (c == other_attr) { skip = true; break; }
//             if (skip) continue;

//             // Get other_val from the current data row.
//             string other_val = "";
//             auto it = data_line.find(other_attr);
//             if (it != data_line.end())
//                 other_val = it->second;

//             // Look up the co-occurrence score for candidate, other_attr, other_val.
//             double score = 0;
//             auto occ_candidate_it = occ_attr.find(candidate);
//             if (occ_candidate_it != occ_attr.end()) {
//                 const auto& other_attr_map = occ_candidate_it->second;
//                 auto occ_other_it = other_attr_map.find(other_attr);
//                 if (occ_other_it != other_attr_map.end()) {
//                     const auto& val_map = occ_other_it->second;
//                     auto score_it = val_map.find(other_val);
//                     if (score_it != val_map.end())
//                         score = score_it->second;
//                 }
//             }
//             cooccur_vec.push_back(score);
//         }
//         double norm_val = euclidean_norm(cooccur_vec) + 1;
//         cooccurance_dist[candidate] = norm_val;
//         total_cooccur += norm_val;
//     }

//     // Combine the base computations with validity checks.
//     for (const auto& candidate : prior) {
//         // Check if candidate is valid: it must pass the AllowNull and pattern requirements.
//         bool valid = false;
//         const auto& info = attr_type.at(attr);
//         string allowNull = info.allowNull;
//         string pattern = info.pattern;  // assume empty string means no pattern
//         if ( (allowNull == "Y" || (allowNull == "N" && candidate != "A Null Cell")) &&
//              (pattern.empty() || std::regex_search(canonical(candidate), std::regex(pattern))) )
//         {
//             valid = true;
//         }
//         if (!valid) {
//             domain_base[candidate] = 0;
//             cooccurance_dist[candidate] = 0;
//             p_obs_G_cand[candidate] = 0;
//         } else {
//             double norm_domain = (total_ground != 0) ? domain_base[candidate] / total_ground : 0;
//             double norm_cooccur = (total_cooccur != 0) ? cooccurance_dist[candidate] / total_cooccur : 0;
//             p_obs_G_cand[candidate] = norm_cooccur;
//         }
//     }
//     return p_obs_G_cand;
// }

// // ----------------- return_penalty_test -----------------

// unordered_map<string, double> CompensativeParameter::return_penalty_test(const string& obs,
//                                                                            const string& attr,
//                                                                            int index,
//                                                                            const Row& data_line,
//                                                                            const vector<string>& prior,
//                                                                            const vector<string>& attr_order) {
//     unordered_map<string, double> p_obs_G_cand;
//     // If no TF-IDF data has been computed for attr, return default scores (all ones).
//     if (tf_idf.find(attr) == tf_idf.end() || tf_idf[attr] == nullptr) {
//         for (const auto& candidate : prior)
//             p_obs_G_cand[candidate] = 1;
//         return p_obs_G_cand;
//     }
//     auto tfidfData = tf_idf[attr];
//     // Build a combined context string using the attributes specified.
//     string obs_combine = "";
//     for (const auto& at : tfidfData->combine_attrs) {
//         auto it = data_line.find(at);
//         if (it != data_line.end())
//             obs_combine += "," + it->second;
//     }
//     // For each candidate ground value, calculate a TF-IDF based score.
//     for (const auto& ground : prior) {
//         string obs_context = ground + obs_combine;
//         int tf = 0;
//         auto it_tf = tfidfData->dic.find(obs_context);
//         if (it_tf != tfidfData->dic.end())
//             tf = it_tf->second;
//         if (tf == 0)
//             continue;
//         int dic_idf_value = 0;
//         auto it_idf = tfidfData->dic_idf.find(obs);
//         if (it_idf != tfidfData->dic_idf.end())
//             dic_idf_value = it_idf->second;
//         double idf = std::log((double)df.rows.size() / (dic_idf_value + 1));
//         if (idf == 0)
//             continue;
//         p_obs_G_cand[ground] = tf * idf;
//     }
//     return p_obs_G_cand;
// }

// // ----------------- Helper for value counts -----------------

// unordered_map<string, int> value_counts(const DataFrame& df, const string& col_name) {
//     unordered_map<string, int> counts;

//     // Step 1: Find the index of the column
//     auto it = std::find(df.columns.begin(), df.columns.end(), col_name);
//     if (it == df.columns.end()) {
//         std::cerr << "Column \"" << col_name << "\" not found in DataFrame.\n";
//         return counts;
//     }

//     int col_index = std::distance(df.columns.begin(), it);

//     // Step 2: Count values from that column in each row
//     for (const auto& row : df.rows) {
//         if (col_index < row.size()) {
//             counts[row[col_index]]++;
//         }
//     }

//     return counts;
// }

// // ----------------- init_tf_idf -----------------

// void CompensativeParameter::init_tf_idf(const vector<string>& attr_order) {
//     for (const auto& pair : attr_type) {
//         string attr = pair.first;
//         vector<string> combine_attrs;
//         // Define local lambdas to retrieve parents and children from the BN model.
//         auto get_parents = [this](const string& node) -> vector<string> {
//             vector<string> parents;
//             for (const auto& p : model.adjacency_list) {
//                 if (p.second.find(node) != p.second.end())
//                     parents.push_back(p.first);
//             }
//             return parents;
//         };
//         auto get_children = [this](const string& node) -> vector<string> {
//             vector<string> children;
//             auto it = model.adjacency_list.find(node);
//             if (it != model.adjacency_list.end()) {
//                 for (const auto& child : it->second)
//                     children.push_back(child);
//             }
//             return children;
//         };

//         // For each attribute in attr_order, check if it is related (as parent or child).
//         for (const auto& at : attr_order) {
//             if (at == attr)
//                 continue;
//             vector<string> parents = get_parents(attr);
//             vector<string> children = get_children(attr);
//             bool related = false;
//             for (const auto& p : parents)
//                 if (p == at) { related = true; break; }
//             for (const auto& c : children)
//                 if (c == at) { related = true; break; }
//             if (related)
//                 combine_attrs.push_back(at);
//         }
//         // If no related attributes, mark this attribute’s TF-IDF data as null.
//         if (combine_attrs.empty()) {                 // fallback
//             for (auto &kv : attr_type)
//                  if (kv.first != attr) combine_attrs.push_back(kv.first);
//         }
//         // Build a new column name by joining combine_attrs with underscores and appending "_TempAttribute".
//         string context_attr;
//         for (size_t i = 0; i < combine_attrs.size(); i++) {
//             context_attr += combine_attrs[i];
//             if (i != combine_attrs.size() - 1)
//                 context_attr += "_";
//         }
//         context_attr += "_TempAttribute";

//         // Make a copy of the DataFrame.
//         DataFrame df_copy = df;

//         // Add context_attr to columns if it doesn't already exist.
//         auto it_col = std::find(df_copy.columns.begin(), df_copy.columns.end(), context_attr);
//         if (it_col == df_copy.columns.end()) {
//             df_copy.columns.push_back(context_attr);
//         }
//         int context_col_idx = std::distance(df_copy.columns.begin(),
//                                             std::find(df_copy.columns.begin(), df_copy.columns.end(), context_attr));

//         // Ensure each row is resized to accommodate the new column
//         for (auto& row : df_copy.rows) {
//             if (row.size() < df_copy.columns.size()) {
//                 row.resize(df_copy.columns.size(), ""); // Fill new entries with empty string
//             }
//         }

//         // Update the context column by concatenating values of combine_attrs
//         for (const auto& at : combine_attrs) {
//             // Find the column index for 'at'
//             auto at_it = std::find(df_copy.columns.begin(), df_copy.columns.end(), at);
//             if (at_it == df_copy.columns.end()) continue; // skip if not found
//             int at_idx = std::distance(df_copy.columns.begin(), at_it);

//             for (auto& row : df_copy.rows) {
//                 if (at_idx < row.size()) {
//                     if (row[context_col_idx].empty()) {
//                         row[context_col_idx] = row[at_idx];
//                     } else {
//                         row[context_col_idx] += "," + row[at_idx];
//                     }
//                 }
//             }
//         }

//         // Create new column name
//         string context_obs = attr + "_" + context_attr;

//         // Add to columns if not already there
//         auto it_obs = std::find(df_copy.columns.begin(), df_copy.columns.end(), context_obs);
//         if (it_obs == df_copy.columns.end()) {
//             df_copy.columns.push_back(context_obs);
//         }
//         int context_obs_idx = std::distance(df_copy.columns.begin(),
//                                             std::find(df_copy.columns.begin(), df_copy.columns.end(), context_obs));

//         // Get column indices for attr and context_attr
//         auto it_attr = std::find(df_copy.columns.begin(), df_copy.columns.end(), attr);
//         auto it_context = std::find(df_copy.columns.begin(), df_copy.columns.end(), context_attr);

//         if (it_attr == df_copy.columns.end() || it_context == df_copy.columns.end()) {
//             std::cerr << "Attribute or context attribute not found in columns.\n";
//             // You could throw an exception here if needed
//             return;
//         }

//         int attr_idx = std::distance(df_copy.columns.begin(), it_attr);
//         int context_idx = std::distance(df_copy.columns.begin(), it_context);

//         // Resize rows and assign values
//         for (auto& row : df_copy.rows) {
//             if (row.size() < df_copy.columns.size()) {
//                 row.resize(df_copy.columns.size(), "");
//             }

//             string val_attr = (attr_idx < row.size()) ? row[attr_idx] : "";
//             string val_context = (context_idx < row.size()) ? row[context_idx] : "";
//             row[context_obs_idx] = val_attr + "," + val_context;
//         }
//         // Compute value counts (term frequencies) for context_obs and for attr.
//         unordered_map<string, int> dic = value_counts(df_copy, context_obs);
//         unordered_map<string, int> dic_idf = value_counts(df_copy, attr);
//         // Store the computed TF-IDF data.
//         std::shared_ptr<TFIDFData> tfdata = std::make_shared<TFIDFData>();
//         tfdata->combine_attrs = combine_attrs;
//         tfdata->dic = dic;
//         tfdata->dic_idf = dic_idf;
//         tf_idf[attr] = tfdata;
//     }
// }

/*********************************************************************
 *  CompensativeParameter.cpp   —   BClean C++ port
 *  ---------------------------------------------------------------
 *  Implements:
 *      • return_penalty
 *      • return_penalty_test   (TF‑IDF)
 *      • init_tf_idf
 *  plus the Levenshtein helper that BClean uses.
 *********************************************************************/

#include "CompensativeParameter.h"   // your class / typedefs
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <regex>
#include <vector>

/*======================================================================
 *  Inline helper: canonical(string)
 *  - lower‑case
 *  - remove spaces and '%' characters
 *====================================================================*/
static inline std::string canonical(std::string s)
{
    std::string out;
    for (char ch : s)
        if (!std::isspace(static_cast<unsigned char>(ch)) && ch != '%')
            out.push_back(std::tolower(static_cast<unsigned char>(ch)));
    return out;
}

/*======================================================================
 *  Levenshtein distance (dynamic‑programming O(N·M))
 *====================================================================*/
int CompensativeParameter::levenshtein_distance(const std::string &a,
                                                const std::string &b)
{
    const size_t n = a.size(), m = b.size();
    std::vector<std::vector<int>> d(n + 1, std::vector<int>(m + 1));
    for (size_t i = 0; i <= n; ++i) d[i][0] = i;
    for (size_t j = 0; j <= m; ++j) d[0][j] = j;

    for (size_t i = 1; i <= n; ++i)
        for (size_t j = 1; j <= m; ++j)
            d[i][j] = std::min({d[i - 1][j] + 1,
                                d[i][j - 1] + 1,
                                d[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1)});
    return d[n][m];
}

/*======================================================================
 *  L2‑norm of a vector<double>
 *====================================================================*/
double CompensativeParameter::euclidean_norm(const std::vector<double> &v)
{
    double s = 0;
    for (double x : v) s += x * x;
    return std::sqrt(s);
}

/*======================================================================
 *  Ctor
 *====================================================================*/
// CompensativeParameter::CompensativeParameter(
//         const std::map<std::string, AttrInfo> &attr_type,
//         const std::unordered_map<std::string,
//                 std::unordered_map<std::string, int>> &domain,
//         const Occur4 &occ,          // Occur4 = 4‑level unordered_map alias
//         const BNGraph &model,
//         const DataFrame &df)
//         : attr_type(attr_type),
//           domain(domain),
//           occurrence(occ),
//           model(model),
//           df(df) {}

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

/*======================================================================
 *  return_penalty
 *====================================================================*/
std::unordered_map<std::string, double>
CompensativeParameter::return_penalty(const std::string &obs,
                                      const std::string &attr,
                                      int /*rowIdx*/,
                                      const Row &row,
                                      const std::vector<std::string> &prior)
{
    std::unordered_map<std::string, double> score;
    if (!occurrence.count(attr)) {
        std::cout << "[DEBUG] Attribute '" << attr << "' not found in occurrence list.\n";
        return score;
    }

    //------------------------------------------------------------
    // canonical observation
    //------------------------------------------------------------
    std::string obs_norm = canonical(
        (obs == "A Null Cell" && attr_type.at(attr).allowNull == "N") ? "" : obs);
    std::cout << "[DEBUG] Normalized observation: " << obs_norm << "\n";

    //------------------------------------------------------------
    // helpers: is the other attribute a parent / child?
    //------------------------------------------------------------
    auto is_related = [&](const std::string &other) -> bool {
        if (model.adjacency_list.count(attr) && model.adjacency_list.at(attr).count(other))
            return true;           // child
        for (auto &kv : model.adjacency_list) {  // parent
            if (kv.second.count(attr) && kv.first == other) return true;
        }
        return false;
    };

    //------------------------------------------------------------
    // Precompute domain & cooccurrence terms
    //------------------------------------------------------------
    double tot_dom = 0, tot_co = 0;
    std::unordered_map<std::string, double> dom_map, co_map;

    std::cout << "[DEBUG] Start computing domain and co-occurrence terms.\n";

    for (const auto &cand_raw : prior) {
        std::string cand = canonical(cand_raw);

        // ----------------- domain term (edit distance) ----------------
        int dist = levenshtein_distance(obs_norm, cand);
        double dom_term = 1 + dist;
        dom_map[cand_raw] = dom_term;
        tot_dom += dom_term;

        std::cout << "  [DEBUG] Candidate: " << cand_raw
                  << ", Canonical: " << cand
                  << ", Edit Distance: " << dist
                  << ", Domain Term: " << dom_term << "\n";

        // ----------------- co-occurrence vector (excluding related attrs) ----------------
        std::vector<double> vec;
        for (const auto &ap : attr_type) {
            const std::string &other = ap.first;
            if (other == attr || is_related(other)) continue;

            std::string other_val = canonical(row.at(other));
            double w = 0;

            if (occurrence.at(attr).count(cand)) {
                auto &other_map = occurrence.at(attr).at(cand);
                if (other_map.count(other)) {
                    auto &value_map = other_map.at(other);
                    if (value_map.count(other_val)) {
                        w = value_map.at(other_val);
                    }
                }
            }
            vec.push_back(w);
            std::cout << "    [DEBUG] Co-Occurrence (" << other << ", " << other_val << ") weight: " << w << "\n";
        }

        double co = euclidean_norm(vec) + 1.0;  // +1 to avoid 0
        co_map[cand_raw] = co;
        tot_co += co;

        std::cout << "  [DEBUG] Candidate: " << cand_raw
                  << ", Co-Occurrence Norm +1: " << co << "\n";
    }

    std::cout << "[DEBUG] Total Domain Sum: " << tot_dom << ", Total Co-Occurrence Sum: " << tot_co << "\n";

    //------------------------------------------------------------
    // Validity check and normalization
    //------------------------------------------------------------
    const auto &meta = attr_type.at(attr);
    for (const auto &cand_raw : prior) {
        bool okNull = meta.allowNull == "Y" || cand_raw != "A Null Cell";
        bool okPat = meta.pattern.empty() ||
                     std::regex_search(canonical(cand_raw),
                                       std::regex(meta.pattern));

        if (!okNull) {
            score[cand_raw] = 0.0;
            std::cout << "[DEBUG] Candidate '" << cand_raw << "' invalid (okNull=" << okNull
                    << ", okPat=" << okPat << "), score=0.\n";
        } else {
            double normalized = (tot_co != 0.0) ? (co_map[cand_raw] / tot_co) : 0.0;

            if (!okPat) {
                normalized *= 0.1; // Penalize but don't zero it
                std::cout << "[DEBUG] Candidate '" << cand_raw << "' soft penalized for pattern mismatch, normalized score: " << normalized << "\n";
            } else {
                std::cout << "[DEBUG] Candidate '" << cand_raw << "' valid, normalized score: " << normalized << "\n";
            }
            score[cand_raw] = normalized;
        }

    }

    std::cout << "[DEBUG] return_penalty finished.\n";
    return score;
}

/*======================================================================
 *  return_penalty_test  (TF‑IDF)
 *====================================================================*/
std::unordered_map<std::string, double>
CompensativeParameter::return_penalty_test(const std::string &obs,
                                           const std::string &attr,
                                           int /*rowIdx*/,
                                           const Row &row,
                                           const std::vector<std::string> &prior,
                                           const std::vector<std::string> &)
{
    std::unordered_map<std::string, double> out;
    if (!tf_idf.count(attr) || tf_idf[attr] == nullptr) {
        for (auto &c : prior) out[c] = 1;
        return out;
    }
    auto tfidf = tf_idf[attr];

    // build comma-separated context values safely
    std::string ctx;
    for (auto &at : tfidf->combine_attrs) {
        auto it = row.find(at);
        if (it != row.end()) {
            ctx += "," + canonical(it->second);
        } else {
            ctx += ",A Null Cell";  // fallback if missing
        }
    }

    for (auto &cand : prior) {
        std::string key = canonical(cand) + ctx;
        int tf = tfidf->dic.count(key) ? tfidf->dic[key] : 0;
        if (!tf) continue;

        int idfc = tfidf->dic_idf.count(canonical(obs))
                   ? tfidf->dic_idf[canonical(obs)] : 0;
        double idf = std::log((double)df.rows.size() / (idfc + 1));
        if (!idf) continue;

        out[cand] = tf * idf;
    }
    return out;
}


/*======================================================================
 *  Helper: count values in a DataFrame column
 *====================================================================*/
static std::unordered_map<std::string, int>
value_counts(const DataFrame &df, size_t colIdx)
{
    std::unordered_map<std::string, int> mp;
    for (const auto &r : df.rows)
        if (colIdx < r.size()) mp[r[colIdx]]++;
    return mp;
}

/*======================================================================
 *  init_tf_idf
 *====================================================================*/
void CompensativeParameter::init_tf_idf(const std::vector<std::string> &order)
{
    for (auto &pr : attr_type) {
        const std::string &attr = pr.first;

        //----------- choose related attributes -----------------------
        std::vector<std::string> comb;
        auto parents = [&](const std::string &n) {
            std::vector<std::string> res;
            for (auto &kv : model.adjacency_list)
                if (kv.second.count(n)) res.push_back(kv.first);
            return res;
        };
        auto children = [&](const std::string &n) {
        std::vector<std::string> res;
        auto it = model.adjacency_list.find(n);
        if (it != model.adjacency_list.end()) {
            for (auto &c : it->second) res.push_back(c);
        }
        return res;
    };

        for (auto &at : order)
            if (at != attr &&
                (std::count(parents(attr).begin(), parents(attr).end(), at) ||
                 std::count(children(attr).begin(), children(attr).end(), at)))
                comb.push_back(at);

        // fallback: at least one other column
        if (comb.empty())
            for (auto &kv : attr_type)
                if (kv.first != attr) comb.push_back(kv.first);

        //--------------------- build context column -----------------
        std::string ctxCol;
        for (size_t i = 0; i < comb.size(); ++i) {
            ctxCol += comb[i];
            if (i + 1 < comb.size()) ctxCol += "_";
        }
        ctxCol += "_TempAttribute";

        DataFrame df2 = df;                     // copy so we can mutate
        if (!std::count(df2.columns.begin(), df2.columns.end(), ctxCol))
            df2.columns.push_back(ctxCol);

        size_t ctxIdx =
            std::find(df2.columns.begin(), df2.columns.end(), ctxCol) -
            df2.columns.begin();
        size_t attrIdx =
            std::find(df2.columns.begin(), df2.columns.end(), attr) -
            df2.columns.begin();

        // fill context column
        for (auto &r : df2.rows) {
            if (r.size() < df2.columns.size()) r.resize(df2.columns.size(), "");
            std::string acc;
            for (auto &at : comb) {
                size_t i = std::find(df2.columns.begin(), df2.columns.end(), at) -
                           df2.columns.begin();
                acc += (acc.empty() ? "" : ",") + canonical(r[i]);
            }
            r[ctxIdx] = acc;
        }

        // build obs column attr_ctx
        std::string obsCol = attr + "_" + ctxCol;
        if (!std::count(df2.columns.begin(), df2.columns.end(), obsCol))
            df2.columns.push_back(obsCol);
        size_t obsIdx =
            std::find(df2.columns.begin(), df2.columns.end(), obsCol) -
            df2.columns.begin();

        for (auto &r : df2.rows) {
            if (r.size() < df2.columns.size()) r.resize(df2.columns.size(), "");
            r[obsIdx] = canonical(r[attrIdx]) + "," + r[ctxIdx];
        }

        //--------------------- compute counts -----------------------
        auto tf = std::make_shared<TFIDFData>();
        tf->combine_attrs = comb;
        tf->dic     = value_counts(df2, obsIdx);
        tf->dic_idf = value_counts(df2, attrIdx);
        tf_idf[attr] = tf;
    }
}

/*======================================================================
 *  (optional) call once in your main() if you want small numbers printed
 *====================================================================*/
// std::cout << std::fixed << std::setprecision(6);
