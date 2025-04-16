#include "../include/Inference.h"
#include "../include/Compensative.h"
#include <iostream>
#include <cmath>
#include <algorithm>

Inference::Inference(const DataMap& dirtyData,
                     const DataMap& processedData,
                     const BNGraph& model,
                     const unordered_map<string, BNGraph>& modelDict,
                     const AttrType& attrType,
                     const unordered_map<string, unordered_map<string,int>>& frequencyList,
                     const unordered_map<string, unordered_map<string, unordered_map<string, unordered_map<string,int>>>>& occurrence1,
                     const shared_ptr<CompensativeParameter>& compParam,
                     const string& inferStrategy,
                     int chunkSize,
                     int numWorker,
                     double tuplePrun,
                     bool debug)
  : dirtyData_(dirtyData),
    data_(processedData),
    model_(model),
    modelDict_(modelDict),
    attrType_(attrType),
    frequencyList_(frequencyList),
    occurrence1_(occurrence1),
    compParam_(compParam),
    inferStrategy_(inferStrategy),
    chunkSize_(chunkSize),
    numWorker_(numWorker),
    tuplePrun_(tuplePrun),
    debug_(debug)
{
    std::cout << "Inference initialized (strategy="
              << inferStrategy_
              << (debug_ ? ", DEBUG=ON)\n" : ")\n");
}

DataMap Inference::repair(const DataMap& /*data*/,
                          const DataFrame& /*cleanData*/,
                          const BNGraph& /*model*/,
                          const AttrType& /*attrType*/)
{

    std::cout << "Starting repair..." << std::endl;

    // Build the list of attributes to consider
    vector<string> nodes;
    for (auto &kv : attrType_) nodes.push_back(kv.first);

    // Copy & fill missing
    DataMap repairData = dirtyData_;
    for (auto &row : repairData) {
        for (auto &n : nodes) {
            if (row.find(n) == row.end() || row[n].empty())
                row[n] = "A Null Cell";
        }
    }

    // Repair every row
    for (size_t i = 0; i < repairData.size(); ++i) {
        auto fixed = repairLine(repairData[i],
                                int(i),
                                model_,
                                modelDict_,
                                nodes,
                                attrType_);
        repairData[i] = std::move(fixed);
        if ((i+1) % 100 == 0)
            std::cout << (i+1) << " rows repaired\n";
    }

    return repairData;
}

Row Inference::repairLine(const Row& dataLine,
                          int line,
                          const BNGraph& /*modelAll*/,
                          const unordered_map<string,BNGraph>& /*modelDict*/,
                          const vector<string>& nodeList,
                          const AttrType& /*attrType*/)
{
    Row repaired = dataLine;

    // 1) Which attrs need repair?
    auto toRepair = prun(dataLine, line, attrType_, nodeList);

    for (auto &attr : toRepair) {
        // 2) Gather candidates from frequencyList_
        vector<string> candidates;
        auto freqIt = frequencyList_.find(attr);
        if (freqIt == frequencyList_.end()) {
            // no data → skip
            continue;
        }
        for (auto &kv : freqIt->second)
            candidates.push_back(kv.first);

        // 3) Precompute parents for this attr
        vector<string> parents;
        auto mdIt = modelDict_.find(attr);
        if (mdIt != modelDict_.end()) {
            for (auto &kv : mdIt->second.adjacency_list) {
                if (kv.second.count(attr))
                    parents.push_back(kv.first);
            }
        }

        struct Cand { string val; double bn, comp, final; };
        vector<Cand> scored;

        // Score every candidate
        for (auto &v : candidates) {
            double bnLog = 0.0;

            if (parents.empty()) {
                // marginal P(attr=v) = freq(v)/sum(freq)
                double freq_v = 0, sum = 0;
                for (auto &pp : freqIt->second) {
                    if (pp.first == v) freq_v = pp.second;
                    sum += pp.second;
                }
                double p = (sum > 0 ? freq_v / sum : 0.0);
                bnLog = std::log(p + 1e-9);
            } else {
                // naive‐Bayes: ∏ P(v | parent = observed)
                for (auto &p : parents) {
                    // observed parent value
                    string pv = "";
                    auto pit = dataLine.find(p);
                    if (pit != dataLine.end())
                        pv = pit->second;

                    // joint count = occurrence1_[attr][v][p][pv], if present
                    double joint = 0.0;
                    auto o1a = occurrence1_.find(attr);
                    if (o1a != occurrence1_.end()) {
                        auto o1v = o1a->second.find(v);
                        if (o1v != o1a->second.end()) {
                            auto o1p = o1v->second.find(p);
                            if (o1p != o1v->second.end()) {
                                auto o1pv = o1p->second.find(pv);
                                if (o1pv != o1p->second.end())
                                    joint = o1pv->second;
                            }
                        }
                    }

                    // parent marginal = frequencyList_[p][pv], if present
                    double pc = 0.0;
                    auto pfl = frequencyList_.find(p);
                    if (pfl != frequencyList_.end()) {
                        auto pit2 = pfl->second.find(pv);
                        if (pit2 != pfl->second.end())
                            pc = pit2->second;
                    }

                    double cond = (pc > 0 ? joint / pc : 0.0);
                    bnLog += std::log(cond + 1e-9);
                }
            }

            // compensative penalty
            auto penMap = compParam_->return_penalty(
                              dataLine.at(attr), 
                              attr,
                              line,
                              dataLine,
                              candidates);
            double compS = 0.0;
            auto itp = penMap.find(v);
            if (itp != penMap.end())
                compS = itp->second;

            double fS = bnLog + compS;
            scored.push_back({ v, bnLog, compS, fS });
        }

        // 4) Sort by descending final score
        std::sort(scored.begin(), scored.end(),
                  [](auto &a, auto &b){ return a.final > b.final; });

        // 5) Debug print
        if (debug_) {
            std::cout << "\n[Row " << line << "] attr='" << attr
                      << "' candidate scores:\n";
            for (auto &c : scored) {
                std::cout
                  << "   " << c.val
                  << "  (BN="    << c.bn
                  << "  COMP="  << c.comp
                  << "  FINAL=" << c.final
                  << ")\n";
            }
        }

        // 6) Pick top
        if (!scored.empty()) {
            repaired[attr] = scored.front().val;
        }
    }

    return repaired;
}

vector<string> Inference::prun(const Row& dataLine,
                               int /*line*/,
                               const AttrType& /*attrType*/,
                               const vector<string>& nodeList)
{
    vector<string> out;
    for (auto &attr : nodeList) {
        auto it = dataLine.find(attr);
        if (it != dataLine.end() && it->second == "A Null Cell")
            out.push_back(attr);
    }
    return out;
}