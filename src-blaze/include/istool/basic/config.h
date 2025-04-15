//
// Created by pro on 2021/12/5.
//

#ifndef ISTOOL_CONFIG_H
#define ISTOOL_CONFIG_H

#include <string>
#include "time_guard.h"
#include "example_space.h"

namespace config {
    extern const std::string KSourcePath;
    extern const int KDefaultSmallVecSize;
    extern bool KIsMultiThread;
    extern const int KIntRange;
}

namespace global {
    extern TimeRecorder recorder;
    extern IOExampleList example_recorder;
}


#endif //ISTOOL_CONFIG_H
