//
// Created by pro on 2024/10/9.
//

#include <mutex>
#include <queue>
#include <thread>

#include "istool/basic/config.h"
#include "istool/fta/fta_multi.h"

using namespace fta;

const int KThreadNum = 4;
const double KSleepTime = 0.01;

namespace {
    struct MergeTask {
        FTA *fta = nullptr;
        int index;
        IOExample example;
        MergeTask(FTA *_fta, int _index, const IOExample &_example)
            : fta(_fta), index(_index), example(_example) {}
        MergeTask() = default;
    };

    FTAList _finalizeFTAs(const std::vector<FoldInfo> &fold_list) {
        FTAList fta_list(fold_list.size());
        for (int i = 0; i < fold_list.size(); ++i)
            fta_list[i] = fold_list[i].fta;
        return fta_list;
    }
} // namespace

FTAList kfold::variants::adaptiveKFold(
    Specification *spec, int fold_num, MultiMergeScheduler *scheduler, int size,
    std::shared_ptr<value::FTAValueSets> fta_value_sets) {
    assert(fold_num);
    auto *grammar = spec->info_list[0]->grammar;
    auto *env = spec->env.get();
    auto init = grammar2FTA(grammar, size, true);
    if (util::isEmpty(init.get()))
        return {};

    auto ref_size = size::getFTASize(init.get());
    std::vector<FoldInfo> fold_list(fold_num);
    for (int i = 0; i < fold_num; ++i)
        fold_list[i] = FoldInfo(init, ref_size, spec->env.get());

    IOExample counter_example;
    IOExampleList used_examples;

    while (!scheduler->isTerminate(fold_list)) {
        int index = 0;
        for (int i = 0; i < fold_list.size(); ++i) {
            if (fold_list[i].size < fold_list[index].size)
                index = i;
        }

        auto &info = fold_list[index];
        if (verify::verifyAfterExcludingExamples(
                info.candidate, spec, counter_example, used_examples)) {
            LOG(WARNING) << "Unexpected fold";
            break;
        }
#ifdef DEBUG
        global::example_recorder.push_back(counter_example);
#endif
        fta_value_sets->initializeValueSet(counter_example);
        used_examples.push_back(counter_example);
        scheduler->startStage(fta::SINGLE_MERGE);
        auto status = info.addExample(env, counter_example, fta_value_sets);
        scheduler->endStage(fta::SINGLE_MERGE);
        if (!status)
            return {};
        LOG(INFO) << "fold " << index << ": " << info.fta->getSizeInfo();
    }
    return _finalizeFTAs(fold_list);
}

FTAList kfold::variants::adaptiveKFoldMulti(
    Specification *spec, int fold_num, MultiMergeScheduler *scheduler, int size,
    std::shared_ptr<value::FTAValueSets> fta_value_sets) {
    assert(fold_num);
    auto *grammar = spec->info_list[0]->grammar;
    auto *env = spec->env.get();
    IOExample counter_example;
    IOExampleList used_examples;

    auto *multi_guard = new MultiThreadTimeGuard();
    std::queue<MergeTask> task_queue;
    std::mutex queue_lock, verifier_lock;
    std::atomic<bool> is_no_solution(false);

    std::vector<FoldInfo> fold_list(fold_num);
    for (int i = 0; i < fold_num; ++i) {
        auto fta = grammar2FTA(grammar, size, true);
        if (util::isEmpty(fta.get()))
            return {};
        auto ref_size = fta::size::getFTASize(fta.get());
        fold_list[i] = FoldInfo(fta, ref_size, spec->env.get());
    }

    auto build_task = [&](int index) -> std::optional<MergeTask> {
        auto &info = fold_list[index];
        if (verify::verifyAfterExcludingExamples(
                info.candidate, spec, counter_example, used_examples)) {
            LOG(WARNING) << "Unexpected fold";
            return {};
        }
        used_examples.push_back(counter_example);
        fta_value_sets->initializeValueSet(counter_example);
        return MergeTask(info.fta.get(), index, counter_example);
    };

    auto thread_func = [&](int thread_index) {
        while (!multi_guard->isTimeout()) {
            queue_lock.lock();
            LOG(INFO) << "thread#" << thread_index << " " << task_queue.size();
            if (task_queue.empty()) {
                queue_lock.unlock();
                return;
            }
            auto task = task_queue.front();
            task_queue.pop();
            queue_lock.unlock();

            auto new_fta = buildFTA(grammar, env, task.example, size, true,
                                    fta_value_sets);
            if (util::isEmpty(new_fta.get())) {
                is_no_solution = true;
                multi_guard->finish();
                return;
            }

            auto res = mergeFTA(task.fta, new_fta.get(), FORWARD, true);
            if (multi_guard->isTimeout()) {
                return;
            }
            if (util::isEmpty(new_fta.get())) {
                is_no_solution = true;
                multi_guard->finish();
                return;
            }

            auto &info = fold_list[task.index];
            info.fta = res;
            info.updateInfo();

            verifier_lock.lock();
            if (scheduler->isTerminate(fold_list)) {
                multi_guard->finish();
                verifier_lock.unlock();
                return;
            }
            auto new_task = build_task(task.index);
            if (!new_task.has_value()) {
                multi_guard->finish();
                verifier_lock.unlock();
                return;
            }
            verifier_lock.unlock();

            queue_lock.lock();
            task_queue.push(new_task.value());
            queue_lock.unlock();
        }
    };

    for (int i = 0; i < fold_num; ++i) {
        auto task = build_task(i);
        if (!task.has_value()) {
            multi_guard->finish();
        } else {
            task_queue.push(task.value());
        }
    }

    std::vector<std::thread> thread_list;
    for (int i = 0; i < KThreadNum; ++i)
        thread_list.emplace_back(thread_func, i);
    for (auto &thread : thread_list)
        thread.join();

#ifdef DEBUG
    for (auto &example : used_examples)
        global::example_recorder.push_back(example);
#endif

    if (is_no_solution)
        return {};
    return _finalizeFTAs(fold_list);
}

FTAList kfold::kFold(Specification *spec, int fold_num,
                     MultiMergeScheduler *scheduler, int size,
                     std::shared_ptr<value::FTAValueSets> fta_value_sets) {
    if (config::KIsMultiThread) {
        return variants::adaptiveKFoldMulti(spec, fold_num, scheduler, size,
                                            fta_value_sets);
    } else {
        return variants::adaptiveKFold(spec, fold_num, scheduler, size,
                                       fta_value_sets);
    }
}

FTAList kfold::variants::prepareSharedUnits(
    Specification *spec, const IOExampleList &examples, int shared_num,
    int size, std::shared_ptr<value::FTAValueSets> fta_value_sets) {
#ifdef DEBUG
    assert(shared_num && shared_num < examples.size());
#endif
    auto *grammar = spec->info_list[0]->grammar;
    fta_value_sets->initializeValueSet(examples[0]);
    auto shared = buildFTA(grammar, spec->env.get(), examples[0], size, true,
                           fta_value_sets);
    if (util::isEmpty(shared.get()))
        return {};
    for (int i = 0; i < shared_num; ++i) {
        fta_value_sets->initializeValueSet(examples[i]);
        auto current = buildFTA(grammar, spec->env.get(), examples[i], size,
                                true, fta_value_sets);
        if (util::isEmpty(current.get()))
            return {};
        shared = mergeFTA(shared.get(), current.get(), FORWARD, true);
        if (util::isEmpty(shared.get()))
            return {};
    }

    FTAList unit_list;
    for (int i = shared_num; i < examples.size(); ++i) {
        fta_value_sets->initializeValueSet(examples[i]);
        auto current = buildFTA(grammar, spec->env.get(), examples[i], size,
                                true, fta_value_sets);
        if (util::isEmpty(current.get()))
            return {};
        auto unit = mergeFTA(shared.get(), current.get(), FORWARD, true);
        if (util::isEmpty(unit.get()))
            return {};
        unit_list.push_back(unit);
    }
    return unit_list;
}

FTAList kfold::variants::prepareSharedUnitsMulti(
    Specification *spec, const IOExampleList &examples, int shared_num,
    int size, std::shared_ptr<value::FTAValueSets> fta_value_sets) {
#ifdef DEBUG
    assert(shared_num && shared_num < examples.size());
#endif
    LOG(INFO) << "Start multi for " << size;
    auto *grammar = spec->info_list[0]->grammar;
    fta_value_sets->initializeValueSet(examples[0]);
    auto shared = buildFTA(grammar, spec->env.get(), examples[0], size, true,
                           fta_value_sets);
    if (util::isEmpty(shared.get()))
        return {};
    for (int i = 0; i < shared_num; ++i) {
        fta_value_sets->initializeValueSet(examples[i]);
        auto current = buildFTA(grammar, spec->env.get(), examples[i], size,
                                true, fta_value_sets);
        if (util::isEmpty(current.get()))
            return {};
        shared = mergeFTA(shared.get(), current.get(), FORWARD, true);
        if (util::isEmpty(shared.get()))
            return {};
    }

    FTAList base_list;
    for (int i = shared_num; i < examples.size(); ++i) {
        fta_value_sets->initializeValueSet(examples[i]);
        auto current = buildFTA(grammar, spec->env.get(), examples[i], size,
                                true, fta_value_sets);
        if (util::isEmpty(current.get()))
            return {};
        base_list.push_back(current);
    }
    FTAList unit_list(base_list.size(), nullptr);
    std::atomic<bool> is_empty = false;
    std::vector<bool> is_free(base_list.size(), true);
    std::mutex lock;

    auto thread = [&]() {
        auto local_shared = fta::util::cloneFTA(shared.get());
        while (!is_empty) {
            lock.lock();
            int index = -1;
            for (int i = 0; i < base_list.size(); ++i) {
                if (is_free[i]) {
                    is_free[i] = false;
                    index = i;
                    break;
                }
            }
            lock.unlock();
            if (index == -1)
                return;
            auto current = fta::mergeFTA(local_shared.get(),
                                         base_list[index].get(), FORWARD, true);
            if (util::isEmpty(current.get())) {
                is_empty = true;
                break;
            }
            lock.lock();
            unit_list[index] = current;
            lock.unlock();
        }
    };
    std::vector<std::thread> thread_list;
    for (int i = 0; i < KThreadNum; ++i) {
        thread_list.emplace_back(thread);
    }
    for (int i = 0; i < KThreadNum; ++i)
        thread_list[i].join();
    if (is_empty)
        return {};
    return unit_list;
}

FTAList
fta::kfold::prepareUnits(Specification *spec, const IOExampleList &examples,
                         int shared_num, int size,
                         std::shared_ptr<value::FTAValueSets> fta_value_sets) {
    if (config::KIsMultiThread) {
        return variants::prepareSharedUnitsMulti(spec, examples, shared_num,
                                                 size, fta_value_sets);
    } else {
        return variants::prepareSharedUnits(spec, examples, shared_num, size,
                                            fta_value_sets);
    }
}