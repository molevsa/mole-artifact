//
// Created by pro on 2024/10/5.
//

#include "istool/fta/fta_multi.h"
#include <unordered_set>
#include "glog/logging.h"
#include "istool/basic/config.h"
#include "istool/fta/fta_merge.h"

using namespace fta;

namespace {
    PProgram _rawCEGIS(Specification* spec, PFTA fta, MergeType type, int size) {
        IOExampleList counter_examples;
        IOExample counter_example;
        auto* env = spec->env.get();
        auto* grammar = spec->info_list[0]->grammar;

        while (!fta::util::isEmpty(fta.get())) {
            auto program = fta::util::extractMinimalProgram(fta.get());
            LOG(INFO) << "CANDIDATE " << program->toString();
#ifdef DEBUG
            for (auto& example: counter_examples) {
                assert(example::satisfyIOExample(program.get(), example, env));
            }
#endif
            if (verify::verifyAfterExcludingExamples(program, spec, counter_example, {})) {
                return program;
            }
            auto new_fta = fta::buildFTA(grammar, env, counter_example, size, true);
            global::node_count += new_fta -> nodeCount();
            global::edge_count += new_fta -> edgeCount();
#ifdef DEBUG
            global::example_recorder.push_back(counter_example);
#endif
            counter_examples.push_back(counter_example);
            
            ++global::example_num;
            LOG(INFO) << "NEW EXAMPLE RAW" << example::ioExample2String(counter_example);
            if (fta::util::isEmpty(new_fta.get())) break;
            fta = fta::mergeFTA(fta.get(), new_fta.get(), type, false);
            global::node_count += fta -> nodeCount();
            global::edge_count += fta -> edgeCount();
            LOG(INFO) << "mid size " << fta->getSizeInfo() << " " << size::getFTASize(fta.get());
            fta->fullyCut();
        }
        return nullptr;
    }
}

PProgram fta::synthesis::rawCEGIS(Specification *spec, MergeType type, int start_size) {
    auto* grammar = spec->info_list[0]->grammar;
    auto* env = spec->env.get();

    auto size_limit = grammar::getMaxSize(grammar);

    for (int size = start_size; size; ++size) {
        LOG(INFO) << "size " << size;
        global::example_num = 0;
        if (size_limit != -1 && size > size_limit) return nullptr;
        auto fta = fta::grammar2FTA(grammar, size, true);
        auto res = _rawCEGIS(spec, fta, type, size);
        if (res) return res;
    }
    assert(0);
}

using fta::size::FTASize;

namespace {
    void _addEmptyLeft(const FTAList& fta_list) {
        auto empty = std::make_shared<EmptyOutputInfo>();
        for (auto& fta: fta_list) {
            for (auto& [_, holder]: fta->node_map) {
                for (auto& list: holder) {
                    for (auto* node: list) {
                        node->oup_info = std::make_shared<BinaryOutputInfo>(empty, node->oup_info);
                    }
                }
            }
        }
    }
}

namespace {
    void _printCompressInfo(FTA* fta, const FTAList& base_list) {
        auto ref = grammar2FTA(fta->grammar, fta->size_limit, true);
        auto ref_size = size::getFTASize(ref.get());
        auto expected_size = ref_size;
        for (auto& base: base_list) {
            auto size = size::getFTASize(base.get());
            expected_size *= size / ref_size;
        }
        auto final_size = size::getFTASize(fta);
        LOG(INFO) << "Final size: " << final_size << ", expected " << expected_size;
    }

    PProgram _kFoldCEGIS(Specification* spec, int fold_num, MultiMergeScheduler* scheduler, MergeType type,int size) {
        auto* grammar = spec->info_list[0]->grammar;
        auto base = grammar2FTA(grammar, size, true);
        scheduler->start(base.get());

        global::recorder.start("pre");
        scheduler->startStage(fta::PRE);
        auto base_list = kfold::kFold(spec, fold_num, scheduler, type, size);
        scheduler->endStage(fta::PRE);
        global::recorder.end("pre");
        if (base_list.empty()) return nullptr;

        _addEmptyLeft(base_list);
        scheduler->startStage(fta::MAIN);
        global::recorder.start("multi");
        PFTA fta;
        if(type == fta::FORWARD)
            fta = fta::merge::mergeFTAForwardMulti(base_list, false);
        else
            fta = fta::merge::mergeFTABackwardMultiWithShare(base_list, false);
        //auto fta = fta::merge::mergeFTAForwardMulti(base_list, false);
        LOG(INFO) << "Raw " << fta->getSizeInfo();
        global::node_count += fta->nodeCount();
        global::edge_count += fta->edgeCount();
        LOG(INFO) << "  " << size::sizeInfo2String(size::getNodeCountVector(fta.get()));
        fta->fullyCut();
        global::recorder.end("multi");
        LOG(INFO) << "Fin " << fta->getSizeInfo();
        LOG(INFO) << "  " << size::sizeInfo2String(size::getNodeCountVector(fta.get()));
        // _printCompressInfo(fta.get(), base_list);

        global::recorder.start("after");
        auto res = _rawCEGIS(spec, fta, fta::FORWARD, size);
        global::recorder.end("after");
        scheduler->endStage(fta::MAIN);
        return res;
    }
}

PProgram fta::synthesis::kFoldCEGIS(Specification *spec, int fold_num, MultiMergeScheduler* scheduler, MergeType type, int start_size) {
    auto* grammar = spec->info_list[0]->grammar;
    int max_size = grammar::getMaxSize(grammar);

    for (int size = start_size;; ++size) {
        global::example_num = 0;
        LOG(INFO) << "Size " << size;
        if (max_size != -1 && size > max_size) return nullptr;
        auto program = _kFoldCEGIS(spec, fold_num, scheduler, type, size);
        if (program) return program;
    }
    assert(0);
}

namespace {
    IOExampleList _collectUsedExamples(FTA* fta) {
        assert(!fta->root_list.empty());
        auto root = fta->root_list[0];
        auto oup_list = root->oup_info->getFullOutput();
        assert(oup_list.size() == fta->info_list.size());
        IOExampleList examples;
        for (int i = 0; i < oup_list.size(); ++i) {
            examples.emplace_back(fta->info_list[i]->param_value, oup_list[i]);
        }
        return examples;
    }

    PFTA _rawMerge(PFTA fta, const IOExampleList& example_list, Env* env, MergeType type) {
        auto* grammar = fta->grammar;
        int size = fta->size_limit;
        for (auto& example: example_list) {
            auto start = buildFTA(grammar, env, example, size, true);
            //assert(!util::isEmpty(fta.get()) && !util::isEmpty(start.get()));
            if(util::isEmpty(fta.get()) || util::isEmpty(start.get()))return nullptr;
            global::recorder.start("multi");
            fta = mergeFTA(fta.get(), start.get(), type, false);
            global::node_count += fta->nodeCount();
            global::edge_count += fta->edgeCount();
            LOG(INFO) << "current " << fta->getSizeInfo();
            fta->fullyCut();
            global::recorder.end("multi");
        }
        return fta;
    }
}

PFTA fta::synthesis::rawMerge(Specification *spec, const IOExampleList &example_list, MergeType type, int size) {
    auto* grammar = spec->info_list[0]->grammar;
    auto* env = spec->env.get();
    auto fta = grammar2FTA(grammar, size, true);
    return _rawMerge(fta, example_list, env, type);
}

PFTA fta::synthesis::kFoldMerge(Specification *spec, const IOExampleList &example_list, int fold_num,
                                  MultiMergeScheduler *scheduler, MergeType type, int size) {
    auto* env = spec->env.get();
    auto func_name = spec->info_list[0]->name;

    auto fio_space = example::buildFiniteIOExampleSpace(example_list, func_name, env);
    auto new_spec = std::make_shared<Specification>(spec->info_list, spec->env, fio_space);
    auto init = grammar2FTA(spec->info_list[0]->grammar, size, true);
    scheduler->start(init.get());
    auto fta_list = kfold::kFold(new_spec.get(), fold_num, scheduler, type, size);
    if (fta_list.empty()) return {};
    for (auto& fold: fta_list) {
        LOG(INFO) << "Fold: " << fold->info_list.size() << " examples with " << fold->getSizeInfo();
    }
    _addEmptyLeft(fta_list);
    PFTA full;
    global::recorder.start("multi");
    if(type == fta::FORWARD)
        full = fta::merge::mergeFTAForwardMulti(fta_list, false);
    else
        full = fta::merge::mergeFTABackwardMultiWithShare(fta_list, false);
    LOG(INFO) << "Raw " << full->getSizeInfo();
    LOG(INFO) << "  " << size::sizeInfo2String(size::getNodeCountVector(full.get()));
    global::node_count += full->nodeCount();
    global::edge_count += full->edgeCount();
    full->fullyCut();
    global::recorder.end("multi");
    LOG(INFO) << "Fin " << full->getSizeInfo();
    LOG(INFO) << "  " << size::sizeInfo2String(size::getNodeCountVector(full.get()));

    std::unordered_set<std::string> used_examples;
    for (auto& fta: fta_list) {
        for (auto& example: _collectUsedExamples(fta.get())) {
            used_examples.insert(example::ioExample2String(example));
        }
    }

    IOExampleList remaining_examples;
    for (auto& example: example_list) {
        auto feature = example::ioExample2String(example);
        if (used_examples.find(feature) == used_examples.end()) {
            remaining_examples.push_back(example);
        }
    }

    return _rawMerge(full, remaining_examples, spec->env.get(), FORWARD);
}

synthesis::BidConfig::BidConfig(int _init, int _shared_num, double _alpha):
    init_example_num(_init), shared_num(_shared_num), alpha(_alpha) {
}

#include "istool/fta/fta_multi.h"
#include "istool/ext/z3/z3_example_space.h"
#include "istool/sygus/theory/basic/clia/clia.h"

PFTA
fta::synthesis::bidMerge(Specification *spec, const IOExampleList &example_list, BidConfig config, int size_limit) {
    IOExampleList init_examples; IOExampleList other_examples;
    for (int i = 0; i < config.init_example_num; ++i) {
        init_examples.push_back(example_list[i]);
    }
    for (int i = config.init_example_num; i < example_list.size(); ++i) {
        other_examples.push_back(example_list[i]);
    }
    auto unit_list = kfold::prepareUnits(spec, init_examples, config.shared_num, size_limit);
    assert(!unit_list.empty());

    auto fta = fta::merge::mergeFTAStagedBidirectional(unit_list, true, config.alpha);
    return _rawMerge(fta, other_examples, spec->env.get(), FORWARD);
}

namespace {
    IOExampleList _getRandomExample(ExampleSpace* example_space, Env* env, int example_num) {
        auto* fio = dynamic_cast<FiniteIOExampleSpace*>(example_space);
        if (fio) {
            IOExampleList example_list;
            for (auto& example: fio->example_space) {
                example_list.push_back(fio->getIOExample(example));
            }
            std::shuffle(example_list.begin(), example_list.end(), env->random_engine);
            if (example_list.size() > example_num) example_list.resize(example_num);
            return example_list;
        }
        auto* zio = dynamic_cast<Z3IOExampleSpace*>(example_space);
        if (zio) {
            IOExampleList example_list;
            std::unordered_set<std::string> known_examples;
            auto gen = [&](Type* type) {
                if (dynamic_cast<TInt*>(type)) {
                    std::uniform_int_distribution<int> dist(-config::KIntRange, config::KIntRange);
                    return BuildData(Int, dist(env->random_engine));
                }
                if (dynamic_cast<TBool*>(type)) {
                    std::bernoulli_distribution dist(0.5);
                    return BuildData(Bool, dist(env->random_engine));
                }
                LOG(FATAL) << "Unknown type " << type->getName();
            };
            for (int _ = 0; _ < (example_num << 2) && example_list.size() < example_num; ++_) {
                Example example;
                for (auto& type: zio->type_list) {
                    example.push_back(gen(type.get()));
                }
                auto io_example = zio->getIOExample(example);
                auto feature = example::ioExample2String(io_example);
                if (known_examples.find(feature) == known_examples.end()) {
                    known_examples.insert(feature);
                    example_list.push_back(io_example);
                }
            }
            return example_list;
        }
        LOG(FATAL) << "Unknown example space";
    }
}

PProgram fta::synthesis::bidCEGIS(Specification *spec, BidConfig config) {
    IOExampleList init_examples = _getRandomExample(spec->example_space.get(), spec->env.get(), config.init_example_num);
    auto* grammar = spec->info_list[0]->grammar;
    int size_limit = grammar::getMaxSize(grammar);
    for (int size = 1;; ++size) {
        LOG(INFO) << "Size " << size;
        if (size_limit != -1 && size > size_limit) return nullptr;
        auto unit_list = fta::kfold::prepareUnits(spec, init_examples, config.shared_num, size);
        for (int i = 0; i < unit_list.size(); ++i) {
            LOG(INFO) << "Unit #" << i << ": " << unit_list[i]->getSizeInfo();
        }
        if (unit_list.empty()) continue;

        auto fta = fta::merge::mergeFTAStagedBidirectional(unit_list, false, config.alpha);
        LOG(INFO) << "Raw merge " << fta->getSizeInfo();
        fta->fullyCut();
        LOG(INFO) << "Fin merge " << fta->getSizeInfo();
        if (fta::util::isEmpty(fta.get())) continue;
        auto res = _rawCEGIS(spec, fta, FORWARD, size);
        if (res) return res;
    }
}