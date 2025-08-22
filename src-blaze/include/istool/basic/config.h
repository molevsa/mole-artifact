//
// Created by pro on 2021/12/5.
//

#ifndef ISTOOL_CONFIG_H
#define ISTOOL_CONFIG_H

#include <string>

#include "example_space.h"
#include "time_guard.h"

namespace config {
extern const std::string KSourcePath;
extern const int KDefaultSmallVecSize;
extern bool KIsMultiThread;
extern const int KIntRange;
}  // namespace config

namespace global {
extern TimeRecorder recorder;
extern IOExampleList example_recorder;
extern int edge_count;
extern int node_count;
extern int example_num;
}  // namespace global

#endif  // ISTOOL_CONFIG_H
