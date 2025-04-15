//
// Created by pro on 2021/12/4.
//

#include "istool/basic/time_guard.h"
#include <iostream>
#include <sys/time.h>
#include <cassert>

TimeGuard::TimeGuard(double _time_limit): time_limit(_time_limit) {
    gettimeofday(&start_time, NULL);
}

double TimeGuard::getPeriod() const {
    timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - start_time.tv_sec) + (now.tv_usec - start_time.tv_usec) / 1e6;
}

double TimeGuard::getRemainTime() const {
    // std::cout << time_limit << " " << getPeriod() << std::endl;
    return time_limit - getPeriod();
}

bool TimeGuard::isTimeout() const {
    return getRemainTime() < 0;
}

void TimeRecorder::start(const std::string &type) {
    gettimeofday(&start_time_map[type], NULL);
}

void TimeRecorder::end(const std::string& type) {
    timeval now; gettimeofday(&now, NULL);
    auto start_time = start_time_map[type];
    auto res = (now.tv_sec - start_time.tv_sec) + (now.tv_usec - start_time.tv_usec) / 1e6;
    value_map[type] += res;
}

double TimeRecorder::query(const std::string &type) {
    if (value_map.find(type) == value_map.end()) return 0.0;
    return value_map[type];
}

void TimeRecorder::printAll() {
    for (auto& info: value_map) {
        std::cout << info.first << ": " << info.second << std::endl;
    }
}

bool MultiThreadTimeGuard::isTimeout() const {
    return is_finished || TimeGuard::isTimeout();
}

void MultiThreadTimeGuard::finish() {
    is_finished = true;
}

MultiThreadTimeGuard::MultiThreadTimeGuard(double time_limit):
    TimeGuard(time_limit), is_finished(false) {
}