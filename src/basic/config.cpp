//
// Created by pro on 2021/12/5.
//

#include "istool/basic/config.h"

#include <sys/time.h>

#include <ctime>

const std::string config::KSourcePath = SOURCEPATH;
const int config::KDefaultSmallVecSize = 64;
bool config::KIsMultiThread = false;

TimeRecorder global::recorder;
IOExampleList global::example_recorder;
int global::edge_count = 0;
int global::node_count = 0;
std::string global::domain = "";
std::map<std::string, int> global::sample_status;
int global::sample_num = 0;
timeval global::start_time;
int global::example_num = 0;
int global::fold_point = 0;
int global::size_counter = 0;
std::vector<int> global::fta_size;

const int config::KDefaultValue = 1000000000;
const int config::KIntMin = -15;
const int config::KIntMax = 15;
int config::KMaxBranchNum = 5;
int config::KTermSolveTurns = 4000;
int config::KTermIntMax = 3;
int config::KTermConfidenceNum = 50;
int config::KRandomC = 10;
int config::KOrLimit = 2;
int config::KSearchTimeLimit = -1;
int config::KFoldNum = 4;