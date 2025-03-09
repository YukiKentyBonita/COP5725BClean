#ifndef BAYESIAN_CLEAN_H
#define BAYESIAN_CLEAN_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <memory>
#include "Dataset.h"
// #include "Compensative.h"
// #include "BN_Structure.h"
// #include "Inference.h"
// #include "CompensativeParameter.h"

class BayesianClean
{
public:
    BayesianClean(std::vector<std::vector<std::string>> dirty_df,
                  std::vector<std::vector<std::string>> clean_df,
                  std::string infer_strategy = "PIPD",
                  double tuple_prun = 1.0,
                  int maxiter = 1,
                  int num_worker = 32,
                  int chunksize = 250,
                  std::string model_path = "",
                  std::string model_save_path = "",
                  std::unordered_map<std::string, std::string> attr_type = {},
                  bool fix_edge = false,
                  std::string model_choice = "");

    void transformData(std::vector<std::vector<std::string>> &data);
    std::vector<std::vector<std::string>> produceTrain(const std::vector<std::vector<std::string>> &data, const std::vector<std::string> &attrs);
    double occurrenceScore(const std::string &pred, const std::string &val, const std::string &attr_main,
                           const std::unordered_map<std::string, std::string> &tuple, int t_id);
    void reRepairAndUpdateParameter(std::vector<std::vector<std::string>> &repair_data, int line,
                                    std::shared_ptr<BayesianNetwork> model, const std::vector<std::string> &nodes_list_sort,
                                    const std::unordered_map<std::string, std::string> &attr_type);
    std::shared_ptr<BayesianNetwork> reconstructNetwork(std::shared_ptr<BayesianNetwork> model,
                                                        const std::vector<std::vector<std::string>> &train_data);

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time, end_time;
    std::vector<std::vector<std::string>> dirty_data;
    std::vector<std::vector<std::string>> clean_data;
    std::unordered_map<std::string, std::string> attr_type;
    bool fix_edge;
    std::string model_path;
    std::string model_save_path;
    std::string model_choice;
    std::string infer_strategy;
    double tuple_prun;
    int maxiter;
    int num_worker;
    int chunksize;

    std::shared_ptr<Dataset> dataLoader;
    std::shared_ptr<Compensative> compensative;
    std::shared_ptr<BN_Structure> structureLearning;
    std::shared_ptr<Inference> inference;
    std::shared_ptr<CompensativeParameter> compensativeParameter;

    std::unordered_map<std::string, std::unordered_map<std::string, int>> occurrenceList;
    std::unordered_map<std::string, int> frequencyList;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> occurrence_1;

    std::vector<std::string> repair_list;
};

#endif // BAYESIAN_CLEAN_H
