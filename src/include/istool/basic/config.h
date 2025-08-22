//
// Created by pro on 2021/12/5.
//

#ifndef ISTOOL_CONFIG_H
#define ISTOOL_CONFIG_H

#include <string>
#include <map>
#include "time_guard.h"
#include "example_space.h"

namespace config {
    extern const std::string KSourcePath;
    extern const int KDefaultSmallVecSize;
    extern bool KIsMultiThread;
    extern const int KIntRange;
    extern const int KDefaultValue;
    extern const int KIntMin;
    extern const int KIntMax;
    extern int KMaxBranchNum;
    extern int KTermSolveTurns;
    extern int KTermIntMax;
    extern int KTermConfidenceNum;
    extern int KRandomC;
    extern int KOrLimit;
    extern int KSearchTimeLimit;
    extern int KFoldNum;
}

namespace global {
    extern TimeRecorder recorder;
    extern IOExampleList example_recorder;
    extern int edge_count;
    extern int node_count;
    extern std::string domain;
    extern std::map<std::string, int> sample_status;
    extern int sample_num;
    extern int example_num;
    extern int fold_point;
    extern timeval start_time;
    extern std::vector<int> fta_size;
    extern int size_counter;
}


#endif //ISTOOL_CONFIG_H
