#ifndef ZKEVM_VERIFIER_MAIN_RECURSIVE_1_HPP
#define ZKEVM_VERIFIER_MAIN_RECURSIVE_1_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "fork_9/calcwit.recursive1.hpp"
#include "fork_9/circom.recursive1.hpp"
#include "fr_goldilocks.hpp"

#include "timer.hpp"
#include <iostream>
#include <unistd.h>
#include "commit_pols_starks.hpp"
using namespace std;

namespace CircomRecursive1Fork9
{
    using json = nlohmann::json;
    Circom_Circuit *loadCircuit(std::string const &datFileName);
    void freeCircuit(Circom_Circuit *circuit);
    void loadJson(Circom_CalcWit *ctx, std::string filename);
    void loadJsonImpl(Circom_CalcWit *ctx, json &j);
    void writeBinWitness(Circom_CalcWit *ctx, std::string wtnsFileName);
    void getBinWitness(Circom_CalcWit *ctx, FrGElement *&pWitness, uint64_t &witnessSize);
    void getCommitedPols(CommitPolsStarks *commitPols, const std::string zkevmVerifier, const std::string execFile, nlohmann::json &zkin, uint64_t N, uint64_t nCols);
    bool check_valid_number(std::string &s, uint base);
}
#endif
