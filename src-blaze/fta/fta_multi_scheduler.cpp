//
// Created by pro on 2024/10/6.
//

#include "istool/fta/fta_multi.h"

using namespace fta;
using size::FTASize;

void FoldInfo::updateInfo() {
    size = fta->edgeCount();
    compress_rate = size::getFTASize(fta.get()) / ref_size;
    example_num = fta->info_list.size();
    candidate = util::extractRandomMinimalProgram(fta.get(), env);
    // candidate = util::extractMinimalProgram(fta.get());
}

FoldInfo::FoldInfo(const PFTA &_fta, FTASize _ref_size, Env *_env)
    : fta(_fta), ref_size(_ref_size), env(_env) {
    updateInfo();
}

bool FoldInfo::addExample(Env *env, const IOExample &example,
                          std::shared_ptr<value::FTAValueSets> fta_value_sets) {
    auto new_fta = buildFTA(fta->grammar, env, example, fta->size_limit, true,
                            fta_value_sets);
    if (util::isEmpty(new_fta.get()))
        return false;
    fta = mergeFTA(fta.get(), new_fta.get(), FORWARD, true);
    if (util::isEmpty(fta.get()))
        return false;
    updateInfo();
    return true;
}

const int KMinSizeLimit = 1e5;

void SizeExpectedScheduler::start(FTA *base) {
    ref_size = size::getFTASize(base);
    expected_size = std::pow(ref_size, alpha);
    if (expected_size < KMinSizeLimit)
        expected_size = KMinSizeLimit;
}

bool SizeExpectedScheduler::isTerminate(
    const std::vector<FoldInfo> &info_list) {
    auto current = ref_size;
    for (auto &info : info_list)
        current *= info.compress_rate;
    if (current <= expected_size)
        return true;
    for (auto &fold : info_list)
        if (fold.example_num < example_limit)
            return false;
    return true;
}

const std::string KPreName = "pre";
const std::string KMainName = "main";
const std::string KSingleName = "single";

namespace {
    std::string _getName(MultiFoldStage stage) {
        switch (stage) {
        case PRE:
            return KPreName;
        case MAIN:
            return KMainName;
        case SINGLE_MERGE:
            return KSingleName;
        }
    }
} // namespace

void TimeAdaptiveScheduler::start(FTA *fta) {
    auto pre_time = recorder.query(KPreName);
    auto main_time = recorder.query(KMainName);
    if (main_time > pre_time) {
        example_num++;
    }
    LOG(INFO) << "example num " << example_num;
    recorder = TimeRecorder();
}

void TimeAdaptiveScheduler::startStage(MultiFoldStage stage) {
    recorder.start(_getName(stage));
}

void TimeAdaptiveScheduler::endStage(MultiFoldStage stage) {
    recorder.end(_getName(stage));
}

bool TimeAdaptiveScheduler::isTerminate(const std::vector<FoldInfo> &info) {
    int current_examples = 0;
    for (auto &fold : info)
        current_examples += fold.example_num;
    return current_examples >= example_num;
}

void TimeLimitScheduler::startStage(MultiFoldStage stage) {
    if (stage != SINGLE_MERGE)
        return;
    is_exceed = false;
    guard = TimeGuard(time_limit);
}

void TimeLimitScheduler::endStage(MultiFoldStage stage) {
    if (stage != SINGLE_MERGE)
        return;
    LOG(INFO) << "Time cost " << guard.getPeriod();
    if (guard.getRemainTime() < 0) {
        is_exceed = true;
    }
}

bool TimeLimitScheduler::isTerminate(const std::vector<FoldInfo> &info) {
    return is_exceed;
}