//
// Created by pro on 2021/12/4.
//

#ifndef ISTOOL_TIME_GUARD_H
#define ISTOOL_TIME_GUARD_H

#include <ctime>
#include <exception>
#include <unordered_map>
#include <string>
#include <atomic>

class TimeGuard {
public:
    timeval start_time;
    double time_limit;
    TimeGuard(double _time_limit);
    virtual bool isTimeout() const;
    double getRemainTime() const;
    double getPeriod() const;
    virtual ~TimeGuard() = default;
};

class TimeRecorder {
public:
    std::unordered_map<std::string, double> value_map;
    std::unordered_map<std::string, timeval> start_time_map;
    TimeRecorder() = default;
    void start(const std::string& type);
    void end(const std::string& type);
    double query(const std::string& type);
    void printAll();
};

class MultiThreadTimeGuard: public TimeGuard {
public:
    std::atomic<bool> is_finished;
    void finish();
    virtual bool isTimeout() const;
    MultiThreadTimeGuard(double time_limit = 1e9);
    ~MultiThreadTimeGuard() = default;
};


#endif //ISTOOL_TIME_GUARD_H
