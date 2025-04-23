#ifndef BAYESIAN_CLEAN_H
#define BAYESIAN_CLEAN_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <memory>
#include "dataset.h"
#include "Compensative.h"
#include "BNStructure.h"
#include "Inference.h"
#include "CompensativeParameter.h"
#include "BayesianNetwork.h"

class BayesianClean
{
public:
    BayesianClean(DataFrame dirty_df,
                  DataFrame clean_df,
                  std::string infer_strategy = "PIPD",
                  double tuple_prun = 1.0,
                  int maxiter = 1,
                  int num_worker = 32,
                  int chunksize = 250,
                  std::string model_path = "",
                  std::string model_save_path = "",
                  std::map<std::string, AttrInfo> attr_type = {},
                  std::vector<Edge> fix_edges = {},
                  std::string model_choice = "");

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time, end_time;
    DataFrame dirty_data;
    DataFrame clean_data;

    DataMap repair_list;
    
    std::map<std::string, AttrInfo> attr_type;
    std::vector<Edge> fix_edge;
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
    std::shared_ptr<CompensativeParameter> compensativeParameter;
    std::shared_ptr<BNStructure> structureLearning;
    std::shared_ptr<Inference> inference;

    std::unordered_map<std::string,
                       std::unordered_map<std::string,
                                          std::unordered_map<std::string,
                                                             std::unordered_map<std::string, double>>>>
        occurrenceList;

    std::unordered_map<std::string,
                       std::unordered_map<std::string, int>>
        frequencyList;

    unordered_map<string,
                  unordered_map<string,
                                unordered_map<string,
                                              unordered_map<string, int>>>>
        occurrence_1;

};

#endif // BAYESIAN_CLEAN_H
