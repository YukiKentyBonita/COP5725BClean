#include "BayesianClean.h"
#include "Compensative.h"
#include "CompensativeParameter.h"
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
    // Create a Dataset loader and preprocess the data
    std::shared_ptr<Dataset> dataLoader = std::make_shared<Dataset>();
    DataFrame processedData = dataLoader->pre_process_data(dirty_data, attr_type);
    std::cout << "+++++++++data loading complete++++++++" << std::endl;

    std::cout << "+++++++++correlation computing++++++++" << std::endl;
    // Create Compensative with the processed DataFrame and attribute types
    dataLoader->print_dataframe(processedData);

    compensative = std::make_shared<Compensative>(processedData, attr_type);
    compensative->build();
    occurrenceList = compensative->getOccurrenceList();
    frequencyList = compensative->getFrequencyList();

    occurrence_1 = compensative->getOccurrence1();
    compensative->printFrequencyList(frequencyList);
    compensative->printOccurrence1(occurrence_1);
    compensative->printOccurrenceList(occurrenceList);

    structureLearning = std::make_shared<BNStructure>(processedData, model_path, model_choice, fix_edge, model_save_path);
    BNResult bn_result = structureLearning->get_bn();
    structureLearning->print_bn_result(bn_result);

    compensativeParameter = std::make_shared<CompensativeParameter>(attr_type,
                                                                    frequencyList,
                                                                    occurrenceList,
                                                                    bn_result.full_graph,
                                                                    processedData);

    std::cout << "\n=========== Running CompensativeParameter Tests ===========\n";

    if (processedData.rows.empty())
    {
        std::cerr << "[Test] No data rows in processedData. Skipping test.\n";
        return;
    }

    int row_index = 0;

    Row row_map;
    const vector<string> &col_names = processedData.columns;
    const vector<string> &row_values = processedData.rows[row_index];
    for (size_t i = 0; i < col_names.size(); ++i)
    {
        if (i < row_values.size())
        {
            row_map[col_names[i]] = row_values[i];
        }
    }

    // === Test 1: return_penalty ===
    std::string test_attr = ""; // choose first valid attr with a non-empty value
    for (const auto &[attr_name, info] : attr_type)
    {
        if (row_map.find(attr_name) != row_map.end() && !row_map[attr_name].empty())
        {
            test_attr = attr_name;
            break;
        }
    }
    if (test_attr.empty())
    {
        std::cerr << "[Test] No suitable attribute found for testing return_penalty.\n";
    }
    std::string obs = row_map[test_attr];
    std::vector<std::string> prior_candidates;
    for (const auto &[val, _] : frequencyList[test_attr])
    {
        prior_candidates.push_back(val);
        if (prior_candidates.size() >= 5)
            break;
    }
    std::cout << "[Test] Testing return_penalty for attribute: " << test_attr << ", observed: " << obs << "\n";
    auto penalty_scores = compensativeParameter->return_penalty(obs, test_attr, row_index, row_map, prior_candidates);

    std::cout << "→ return_penalty output:\n";
    for (const auto &[cand, score] : penalty_scores)
    {
        std::cout << "  " << cand << ": " << score << "\n";
    }

    // === Test 2: init_tf_idf ===
    std::cout << "\n[Test] Initializing TF-IDF structure...\n";
    compensativeParameter->init_tf_idf(col_names);

    // === Test 3: return_penalty_test ===
    std::cout << "[Test] Testing return_penalty_test for attribute: " << test_attr << "\n";
    auto penalty_scores_tfidf = compensativeParameter->return_penalty_test(
        obs, test_attr, row_index, row_map, prior_candidates, col_names);

    std::cout << "→ return_penalty_test output (TF-IDF):\n";
    for (const auto &[cand, score] : penalty_scores_tfidf)
    {
        std::cout << "  " << cand << ": " << score << "\n";
    }

    std::cout << "\n=========== CompensativeParameter Tests Complete ===========\n";

    // --- convert DataFrame → DataMap
    using Row = std::unordered_map<std::string, std::string>;
    using DataMap = std::vector<Row>;

    DataMap dirtyMap, processedMap;
    // build dirtyMap
    for (auto &vals : dirty_data.rows)
    {
        Row r;
        for (size_t j = 0; j < dirty_data.columns.size(); ++j)
            r[dirty_data.columns[j]] = vals[j];
        dirtyMap.push_back(std::move(r));
    }
    // build processedMap
    for (auto &vals : processedData.rows)
    {
        Row r;
        for (size_t j = 0; j < processedData.columns.size(); ++j)
            r[processedData.columns[j]] = vals[j];
        processedMap.push_back(std::move(r));
    }

    inference = std::make_shared<Inference>(
        /*dirtyData*/ dirtyMap,
        /*processedData*/ processedMap,
        /*model*/ bn_result.full_graph,
        /*modelDict*/ bn_result.partition_graphs,
        /*attrType*/ attr_type,
        /*frequencyList*/ frequencyList,
        /*occurrence1*/ occurrence_1,
        /*compParam*/ compensativeParameter,
        /*strategy*/ infer_strategy,
        /*chunkSize*/ chunksize,
        /*numWorker*/ num_worker,
        /*tuplePrun*/ tuple_prun,
        true);

    repair_list = inference->repair(processedMap, clean_data, bn_result.full_graph, attr_type);
    end_time = std::chrono::high_resolution_clock::now();
}