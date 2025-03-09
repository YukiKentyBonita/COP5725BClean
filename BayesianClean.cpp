#include "BayesianClean.h"
#include <algorithm>
#include <numeric>
#include "dataset.h"

BayesianClean::BayesianClean(DataFrame dirty_df, DataFrame clean_df,
                             std::string infer_strategy,
                             double tuple_prun,
                             int maxiter,
                             int num_worker,
                             int chunksize,
                             std::string model_path,
                             std::string model_save_path,
                             map<string, AttrInfo> attr_type,
                             bool fix_edge,
                             std::string model_choice)
    : dirty_data(dirty_df), clean_data(clean_df), infer_strategy(infer_strategy),
      tuple_prun(tuple_prun), maxiter(maxiter), num_worker(num_worker),
      chunksize(chunksize), model_path(model_path), model_save_path(model_save_path),
      attr_type(attr_type), fix_edge(fix_edge), model_choice(model_choice)
{

    start_time = std::chrono::high_resolution_clock::now();

    std::cout << "+++++++++data loading++++++++" << std::endl;
    dataLoader = std::make_shared<Dataset>();
    dirty_data = dirty_df;
    clean_data = clean_df;

    auto data = dataLoader->pre_process_data(dirty_data, attr_type);
    std::cout << "+++++++++data loading complete++++++++" << std::endl;

    std::cout << "+++++++++computing error cell++++++++" << std::endl;
    auto actual_error = dataLoader->get_error(dirty_data, clean_data);
    std::cout << "error: " << actual_error.size() << std::endl;
    std::cout << "+++++++++error cell computing complete++++++++" << std::endl;

    std::cout << "+++++++++correlation computing++++++++" << std::endl;
    compensative = std::make_shared<Compensative>(data, attr_type);
    auto result = compensative->build();
    occurrenceList = result.occurrenceList;
    frequencyList = result.frequencyList;
    occurrence_1 = result.occurrence_1;
    std::cout << "+++++++++correlation computing complete++++++++" << std::endl;

    structureLearning = std::make_shared<BN_Structure>(data, model_path, model_save_path, model_choice, fix_edge);
    auto bn_result = structureLearning->getBN();

    compensativeParameter = std::make_shared<CompensativeParameter>(attr_type, frequencyList, occurrenceList, bn_result.model, data);

    inference = std::make_shared<Inference>(dirty_data, data, bn_result.model, bn_result.model_dict, attr_type,
                                            frequencyList, occurrence_1, compensativeParameter, infer_strategy);

    repair_list = inference->repair(data, clean_data, bn_result.model, attr_type);

    end_time = std::chrono::high_resolution_clock::now();
}

void BayesianClean::transformData(std::vector<std::vector<std::string>> &data)
{
    for (auto &row : data)
    {
        for (auto &val : row)
        {
            if (val.empty())
            {
                val = "A Null Cell";
            }
        }
    }
}

std::vector<std::vector<std::string>> BayesianClean::produceTrain(const std::vector<std::vector<std::string>> &data, const std::vector<std::string> &attrs)
{
    std::vector<std::vector<std::string>> filtered_data;
    for (const auto &row : data)
    {
        std::vector<std::string> filtered_row;
        for (const auto &attr : attrs)
        {
            auto it = std::find(attrs.begin(), attrs.end(), attr);
            if (it != attrs.end())
            {
                filtered_row.push_back(row[std::distance(attrs.begin(), it)]);
            }
        }
        filtered_data.push_back(filtered_row);
    }
    return filtered_data;
}

double BayesianClean::occurrenceScore(const std::string &pred, const std::string &val, const std::string &attr_main,
                                      const std::unordered_map<std::string, std::string> &tuple, int t_id)
{
    double P_Ai = 1.0;
    std::string candidate = val.substr(attr_main.size() + 1);

    for (const auto &pair : tuple)
    {
        if (pair.first == attr_main)
            continue;

        std::string val_vice = pair.second;
        int occur = occurrenceList[attr_main][candidate][pair.first].count(val_vice) ? occurrenceList[attr_main][candidate][pair.first][val_vice] : 0;
        P_Ai *= (occur / (double)frequencyList[attr_main][candidate]);

        if (P_Ai == 0.0)
            break;
    }
    return P_Ai;
}

std::shared_ptr<BayesianNetwork> BayesianClean::reconstructNetwork(std::shared_ptr<BayesianNetwork> model,
                                                                   const std::vector<std::vector<std::string>> &train_data)
{
    auto next_iter_model = std::make_shared<BayesianNetwork>();
    // Implementation for BayesianNetwork reconstruction...
    return next_iter_model;
}
