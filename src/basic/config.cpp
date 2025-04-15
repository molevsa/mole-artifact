//
// Created by pro on 2021/12/5.
//

#include "istool/basic/config.h"

#include <sys/time.h>

#include <ctime>

const std::string config::KSourcePath = "/path/to/mole";
const int config::KDefaultSmallVecSize = 64;
bool config::KIsMultiThread = false;

TimeRecorder global::recorder;
IOExampleList global::example_recorder;
int global::edge_count = 0;
int global::node_count = 0;
timeval global::start_time;
int global::example_num = 0;

const int config::KIntMin = -15;
const int config::KIntMax = 15;