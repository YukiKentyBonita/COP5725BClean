#include "CompensativeParameter.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <regex>
#include <vector>

// Remove spaces and '%' characters
static inline std::string canonical(std::string s)
{
    std::string out;
    for (char ch : s)
        if (!std::isspace(static_cast<unsigned char>(ch)) && ch != '%')
            out.push_back(std::tolower(static_cast<unsigned char>(ch)));
    return out;
}

 // Levenshtein distance
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

// L2‑norm of a vector<double>
double CompensativeParameter::euclidean_norm(const std::vector<double> &v)
{
    double s = 0;
    for (double x : v) s += x * x;
    return std::sqrt(s);
}

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

std::unordered_map<std::string, double>
CompensativeParameter::return_penalty(const std::string &obs,
                                      const std::string &attr,
                                      int /*rowIdx*/,
                                      const Row &row,
                                      const std::vector<std::string> &prior)
{
    using std::string;
    std::unordered_map<string,double> score;                 // result
    if (!occurrence.count(attr)) {
        std::cout << "[DEBUG] Attribute '" << attr << "' not in occurrence list\n";
        return score;
    }

    string obs_norm = canonical(
        (obs == "A Null Cell" && attr_type.at(attr).allowNull == "N") ? "" : obs);
    std::cout << "[DEBUG] Normalized observation: " << obs_norm << '\n';

    auto is_related = [&](const string &other)->bool {
        if (model.adjacency_list.count(attr) &&
            model.adjacency_list.at(attr).count(other))          // child
            return true;
        for (auto &kv : model.adjacency_list)                    // parent
            if (kv.second.count(attr) && kv.first == other) return true;
        return false;
    };

    // Compute a raw compensative score per candidate
    std::unordered_map<string,double> raw_map;
    double tot_raw = 0.0;

    for (const auto &cand_raw : prior) {
        const string cand_norm = canonical(cand_raw);

        //---------------- domain distance -----------------
        int dist = levenshtein_distance(obs_norm, cand_norm);
        double dom_term = 1.0 + dist;

        //---------------- co‑occurrence -------------------
        std::vector<double> vec;
        for (const auto &ap : attr_type) {
            const string &other = ap.first;
            if (other == attr || is_related(other)) continue;

            const std::string other_val_raw = row.at(other);
            const std::string other_val     = canonical(other_val_raw);

            double w = 0.0;
            auto occ_cand_it = occurrence.at(attr).find(cand_norm);
            if (occ_cand_it != occurrence.at(attr).end()) {
                auto oth_it = occ_cand_it->second.find(other);
                if (oth_it != occ_cand_it->second.end()) {
                    auto val_it = oth_it->second.find(other_val);
                    if (val_it != oth_it->second.end())
                        w = val_it->second;
                }
            }
            vec.push_back(w);
            std::cout << "    [DEBUG] Co-Occurrence (" << other << ", "
                      << other_val << ")" << '\n';
        }

        constexpr double GAMMA = 1.5;
        double co_norm = euclidean_norm(vec);          // avoid 0
        double raw     = std::pow(1.0 + co_norm,  GAMMA) / std::pow(1.0 + dist,      1.0);               // <-- merge
        raw_map[cand_raw] = raw;
        tot_raw += raw;

        std::cout << "  [DEBUG] Candidate: "   << cand_raw
                  << ", Canonical: "           << cand_norm
                  << ", Edit Distance: "       << dist
                  << ", Domain Term: "         << dom_term
                  << "\n  [DEBUG] Candidate: " << cand_raw << '\n';
    }

    // Validity / pattern check  +  normalisation
    const auto &meta = attr_type.at(attr);
    for (const auto &cand_raw : prior) {
        bool okNull = meta.allowNull == "Y" || cand_raw != "A Null Cell";
        bool okPat  = meta.pattern.empty() ||
                      std::regex_search(canonical(cand_raw),
                                         std::regex(meta.pattern));

        double comp = tot_raw ? raw_map[cand_raw] / tot_raw : 0.0;

        if (!okNull) {
            comp = 0.0;
            std::cout << "[DEBUG] Candidate '" << cand_raw
                      << "' invalid (okNull=0, okPat=" << okPat << "), score=0.\n";
        } else if (!okPat) {
            comp *= 0.1;
            std::cout << "[DEBUG] Candidate '" << cand_raw
                      << "' soft penalized for pattern mismatch, normalized score: "
                      << comp << '\n';
        } else {
            std::cout << "[DEBUG] Candidate '" << cand_raw
                      << "' valid, normalized score: " << comp << '\n';
        }
        score[cand_raw] = comp;
    }

    std::cout << "[DEBUG] return_penalty finished.\n";
    return score;
}

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

    // Build comma-separated context values safely
    std::string ctx;
    for (auto &at : tfidf->combine_attrs) {
        auto it = row.find(at);
        if (it != row.end()) {
            ctx += "," + canonical(it->second);
        } else {
            ctx += ",A Null Cell"; 
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

// Helper: count values in a DataFrame column
static std::unordered_map<std::string, int>
value_counts(const DataFrame &df, size_t colIdx)
{
    std::unordered_map<std::string, int> mp;
    for (const auto &r : df.rows)
        if (colIdx < r.size()) mp[r[colIdx]]++;
    return mp;
}

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

        DataFrame df2 = df;               
        if (!std::count(df2.columns.begin(), df2.columns.end(), ctxCol))
            df2.columns.push_back(ctxCol);

        size_t ctxIdx =
            std::find(df2.columns.begin(), df2.columns.end(), ctxCol) -
            df2.columns.begin();
        size_t attrIdx =
            std::find(df2.columns.begin(), df2.columns.end(), attr) -
            df2.columns.begin();

        // Fill context column
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

        // Build obs column
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
