#include "BayesianClean.h"
#include "Compensative.h"
#include "dataset.h"
#include <iostream>
#include <memory>

BayesianClean::BayesianClean(DataFrame dirty_df, DataFrame clean_df,
                             string infer_strategy,
                             double tuple_prun,
                             int maxiter,
                             int num_worker,
                             int chunksize,
                             string model_path,
                             string model_save_path,
                             map<string, AttrInfo> attr_type,
                             vector<Edge> fix_edge,
                             string model_choice)
    : dirty_data(dirty_df), clean_data(clean_df), infer_strategy(infer_strategy),
      tuple_prun(tuple_prun), maxiter(maxiter), num_worker(num_worker),
      chunksize(chunksize), model_path(model_path), model_save_path(model_save_path),
      attr_type(attr_type), fix_edge(fix_edge), model_choice(model_choice)
{
  std::cout << "+++++++++data loading++++++++" << std::endl;
  // Create a Dataset loader and preprocess the data.
  std::shared_ptr<Dataset> dataLoader = std::make_shared<Dataset>();
  DataFrame processedData = dataLoader->pre_process_data(dirty_data, attr_type);
  std::cout << "+++++++++data loading complete++++++++" << std::endl;

  std::cout << "+++++++++correlation computing++++++++" << std::endl;
  // Create Compensative with the processed DataFrame and attribute types.
  dataLoader->print_dataframe(processedData);

  compensative = std::make_shared<Compensative>(processedData, attr_type);
  compensative->build();
  occurrenceList = compensative->getOccurrenceList();
  frequencyList = compensative->getFrequencyList();

  // Now that occurrence_1 in BayesianClean is declared as a 4-level map,
  // the assignment below is valid.
  occurrence_1 = compensative->getOccurrence1();
  // std::cout << "+++++++++correlation computing complete++++++++" << std::endl;

  structureLearning = std::make_shared<BNStructure>(processedData, model_path, model_choice, fix_edge, model_save_path);
  BNResult bn_result = structureLearning->get_bn();
  structureLearning->print_bn_result(bn_result);

  // compensativeParameter = std::make_shared<CompensativeParameter>(attr_type, frequencyList, occurrenceList, bn_result.model, data);

  // inference = std::make_shared<Inference>(dirty_data, data, bn_result.model, bn_result.model_dict, attr_type,
  //                                         frequencyList, occurrence_1, compensativeParameter, infer_strategy);

  // repair_list = inference->repair(data, clean_data, bn_result.model, attr_type);

  // end_time = std::chrono::high_resolution_clock::now();
}

// void BayesianClean::transformData(std::vector<std::vector<std::string>> &data)
// {
//     for (auto &row : data)
//     {
//         for (auto &val : row)
//         {
//             if (val.empty())
//             {
//                 val = "A Null Cell";
//             }
//         }
//     }
// }

// std::vector<std::vector<std::string>> BayesianClean::produceTrain(const std::vector<std::vector<std::string>> &data, const std::vector<std::string> &attrs)
// {
//     std::vector<std::vector<std::string>> filtered_data;
//     for (const auto &row : data)
//     {
//         std::vector<std::string> filtered_row;
//         for (const auto &attr : attrs)
//         {
//             auto it = std::find(attrs.begin(), attrs.end(), attr);
//             if (it != attrs.end())
//             {
//                 filtered_row.push_back(row[std::distance(attrs.begin(), it)]);
//             }
//         }
//         filtered_data.push_back(filtered_row);
//     }
//     return filtered_data;
// }

// double BayesianClean::occurrenceScore(const std::string &pred, const std::string &val, const std::string &attr_main,
//                                       const std::unordered_map<std::string, std::string> &tuple, int t_id)
// {
//     double P_Ai = 1.0;
//     std::string candidate = val.substr(attr_main.size() + 1);

//     for (const auto &pair : tuple)
//     {
//         if (pair.first == attr_main)
//             continue;

//         std::string val_vice = pair.second;
//         int occur = occurrenceList[attr_main][candidate][pair.first].count(val_vice) ? occurrenceList[attr_main][candidate][pair.first][val_vice] : 0;
//         P_Ai *= (occur / (double)frequencyList[attr_main][candidate]);

//         if (P_Ai == 0.0)
//             break;
//     }
//     return P_Ai;
// }

// std::shared_ptr<BayesianNetwork> BayesianClean::reconstructNetwork(std::shared_ptr<BayesianNetwork> model,
//                                                                    const std::vector<std::vector<std::string>> &train_data)
// {
//     auto next_iter_model = std::make_shared<BayesianNetwork>();
//     // Implementation for BayesianNetwork reconstruction...
//     return next_iter_model;
// }
