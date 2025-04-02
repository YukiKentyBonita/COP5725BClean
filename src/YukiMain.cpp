/* FOR TESTING Compensative */

// #include "../dataset.h"     // For DataFrame and AttrInfo
// #include "../include/Compensative.h"
// #include <iostream>
// #include <map>

// using namespace std;

// // Helper to convert DataFrame to Compensative::Data
// Compensative::Data convert_to_internal(const DataFrame& df) {
//     Compensative::Data internal_data;
//     for (const auto& row : df.rows) {
//         Compensative::Row r;
//         for (size_t i = 0; i < df.columns.size(); ++i) {
//             r[df.columns[i]] = row[i];
//         }
//         internal_data.push_back(r);
//     }
//     return internal_data;
// }

// // Main testing function
// int main() {
//     // Step 1: Create small hardcoded DataFrame
//     DataFrame test_df;
//     test_df.columns = {"Name", "City", "Country"};
//     test_df.rows = {
//         {"Alice", "New York", "USA"},
//         {"Bob", "London", "UK"},
//         {"Alice", "New York", "USA"},
//         {"Eve", "Paris", "France"},
//         {"Bob", "London", "UK"}
//     };

//     // Step 2: Define attribute types
//     map<string, AttrInfo> attr_type = {
//         {"Name", {"", "String"}},     // No regex pattern
//         {"City", {"", "String"}},
//         {"Country", {"", "String"}}
//     };

//     // Step 3: Convert test_df to Compensative format
//     Compensative::Data internal_data = convert_to_internal(test_df);

//     // Step 4: Convert attr_type to Compensative::AttrType
//     Compensative::AttrType comp_attr_type;
//     for (const auto& [key, val] : attr_type) {
//         comp_attr_type[key]["AllowNull"] = "N";  // Disallow Nulls
//         comp_attr_type[key]["pattern"] = val.pattern;
//     }

//     // Step 5: Run Compensative
//     Compensative comp(internal_data, comp_attr_type);
//     comp.build();

//     // Step 6: Output FrequencyList
//     cout << "\n===== Frequency List =====\n";
//     for (const auto& [attr, val_map] : comp.getFrequencyList()) {
//         cout << attr << ":\n";
//         for (const auto& [val, freq] : val_map) {
//             cout << "  " << val << ": " << freq << "\n";
//         }
//     }

//     // Step 7: Output OccurrenceList summary
//     cout << "\n===== Occurrence List =====\n";
//     for (const auto& [attr, val_map] : comp.getOccurrenceList()) {
//         cout << "Attribute: " << attr << "\n";
//         for (const auto& [val, submap] : val_map) {
//             cout << "  Value: " << val << "\n";
//             for (const auto& [other_attr, other_vals] : submap) {
//                 for (const auto& [other_val, score] : other_vals) {
//                     cout << "    " << other_attr << " = " << other_val
//                          << " → Score: " << score << "\n";
//                 }
//             }
//         }
//     }

//     // Step 8: Output Occurrence1 summary
//     cout << "\n===== Occurrence_1 =====\n";
//     for (const auto& [attr, val_map] : comp.getOccurrence1()) {
//         cout << "Attribute: " << attr << "\n";
//         for (const auto& [val, submap] : val_map) {
//             cout << "  Value: " << val << "\n";
//             for (const auto& [other_attr, other_vals] : submap) {
//                 for (const auto& [other_val, count] : other_vals) {
//                     cout << "    " << other_attr << " = " << other_val
//                          << " → Count: " << count << "\n";
//                 }
//             }
//         }
//     }

//     return 0;
// }


#include "../include/CompensativeParameter.h"
#include "../include/StaticBayesianModel.h"
#include <iostream>

int main() {
    // Step 1: Define test DataFrame
    DataFrame df;
    df.columns = {"Name", "City", "Country"};
    df.rows = {
        {"Alice", "New York", "USA"},
        {"Bob",   "London",    "UK"},
        {"Eve",   "Paris",     "France"},
        {"Alice", "New York",  "USA"}
    };

    // Step 2: Convert DataFrame to internal Row-based format
    Data data;
    for (const auto& row : df.rows) {
        Row r;
        for (size_t i = 0; i < df.columns.size(); ++i) {
            r[df.columns[i]] = row[i];
        }
        data.push_back(r);
    }

    // Step 3: Attribute metadata
    map<string, AttrInfo> attr_type = {
        {"Name", {"", "String", "N"}},
        {"City", {"", "String", "N"}},
        {"Country", {"", "String", "N"}}
    };

    // Step 4: Domain values for "Name"
    map<string, vector<string>> domain = {
        {"Name", {"Alice", "Bob", "Eve"}}
    };

    // Step 5: Occurrence structure — make sure both City & Country are inserted
    unordered_map<string, unordered_map<string, unordered_map<string, unordered_map<string, double>>>> occurrence;

    occurrence["Name"]["Alice"]["City"]["New York"] = 18;
    occurrence["Name"]["Alice"]["Country"]["USA"] = 18;

    occurrence["Name"]["Bob"]["City"]["London"] = 9;
    occurrence["Name"]["Bob"]["Country"]["UK"] = 9;

    occurrence["Name"]["Eve"]["City"]["Paris"] = 9;
    occurrence["Name"]["Eve"]["Country"]["France"] = 9;

    // Step 6: Bayesian model structure
    std::shared_ptr<BayesianModel> model = std::make_shared<StaticBayesianModel>();

    // Step 7: Create CompensativeParameter
    CompensativeParameter cp(attr_type, domain, occurrence, model, data);

    // Step 8: Test Bob (row index 1)
    const Row& test_row = data[1];  // Bob
    string observed_val = "Bob";
    vector<string> prior_candidates = {"Alice", "Bob", "Eve"};

    std::cout << "=== Occurrence Keys for Name → Bob ===" << std::endl;
for (const auto& attr_pair : occurrence["Name"]["Bob"]) {
    std::cout << "  Attr: " << attr_pair.first << std::endl;
    for (const auto& val_pair : attr_pair.second) {
        std::cout << "    Val: " << val_pair.first
                  << ", Score: " << val_pair.second << std::endl;
    }
}


    // Step 9: Run penalty scoring
    map<string, double> penalty_scores = cp.return_penalty(observed_val, "Name", 1, test_row, prior_candidates);

    // Step 10: Print results
    std::cout << "Penalty scores for Name='Bob' at row 1:\n";
    for (const auto& [val, score] : penalty_scores) {
        std::cout << "  Prior: " << val << " → Score: " << score << "\n";
    }

    return 0;
}
