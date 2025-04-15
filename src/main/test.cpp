//
// Created by pro on 2024/9/18.
//

#include "istool/fta/fta_multi.h"
#include "istool/fta/fta_merge.h"
#include "istool/basic/config.h"
#include "istool/basic/verifier.h"
#include "istool/sygus/parser/parser.h"
#include "istool/ext/z3/z3_example_space.h"
#include "istool/ext/z3/z3_verifier.h"
#include "istool/sygus/theory/basic/clia/clia.h"
#include "glog/logging.h"
#include <random>
#include <unordered_set>

Verifier* _getVerifier(ExampleSpace* example_space) {
    auto* finite_io_space  = dynamic_cast<FiniteExampleSpace*>(example_space);
    if (finite_io_space) {
        return new FiniteExampleVerifier(finite_io_space);
    }
    auto* z3_space = dynamic_cast<Z3ExampleSpace*>(example_space);
    if (z3_space) {
        return new Z3Verifier(z3_space);
    }
    LOG(FATAL) << "Unsupported example space";
}


IOExampleList generateExamples(Z3ExampleSpace* example_space, int example_num, int int_range, Env* env) {
    bool is_all_bool = true;
    for (auto& inp_type: example_space->type_list) {
        if (!dynamic_cast<TBool*>(inp_type.get())) {
            is_all_bool = false;
        }
    }
    int n = example_space->type_list.size();
    ExampleList raw_examples;
    if (is_all_bool && n <= 7) {
        for (int state = 0; state < (1 << n); ++state) {
            DataList example(n);
            for (int i = 0; i < n; ++i) {
                if (state & (1 << i)) example[i] = BuildData(Bool, true); else example[i] = BuildData(Bool, false);
            }
            raw_examples.push_back(example);
        }
    } else {
        auto gen = [=](Type* type) {
            if (dynamic_cast<TBool*>(type)) {
                auto bool_dis = std::bernoulli_distribution(0.5);
                return BuildData(Bool, bool_dis(env->random_engine));
            }
            if (dynamic_cast<TInt*>(type)) {
                auto int_dis = std::uniform_int_distribution<int>(-int_range, int_range);
                return BuildData(Int, int_dis(env->random_engine));
            }
            LOG(FATAL) << "Unknown type " << type->getName();
        };
        std::unordered_set<std::string> known_examples;
        for (int _ = 0; _ < example_num; ++_) {
            DataList example(n);
            for (int i = 0; i < n; ++i) example[i] = gen(example_space->type_list[i].get());
            auto feature = data::dataList2String(example);
            if (known_examples.find(feature) != known_examples.end()) continue;
            known_examples.insert(feature);
            raw_examples.push_back(example);
        }
    }

    std::shuffle(raw_examples.begin(), raw_examples.end(), env->random_engine);
    IOExampleList examples;
    auto* io_space = dynamic_cast<IOExampleSpace*>(example_space);
    assert(io_space);
    for (auto& example: raw_examples) examples.push_back(io_space->getIOExample(example));
    return examples;
}

const int KExampleNum = 1000;
const int KIntRange = 10;

void printResult(const std::vector<std::string>& info_list, const std::string& path) {
    for (auto& info: info_list) std::cout << info << std::endl;
    if (!path.empty()) {
        auto f = std::fopen(path.c_str(), "w");
        for (auto& info: info_list) {
            fprintf(f, "%s\n", info.c_str());
        }
        fprintf(f, "%.5f\n", global::recorder.query("total"));
        fclose(f);
    }
}

int main(int argv, char** argc) {
    std::string task_path =
            "/path/to/mole/benchmark/circuit/hd05.eqn_sygus_iter_120_0.sl";
    // std::string task_path = config::KSourcePath + "/tests/max3.sl";
    int start_size = 35, fold_num = 5;
    config::KIsMultiThread = true;

    auto* spec = parser::getSyGuSSpecFromFile(task_path);
    auto name = spec->info_list[0]->name;
    LOG(INFO) << "start";

    auto* z3_space = dynamic_cast<Z3ExampleSpace*>(spec->example_space.get());
    if (z3_space) {
        auto examples = generateExamples(z3_space, KExampleNum, KIntRange, spec->env.get());
        spec->example_space = example::buildFiniteIOExampleSpace(examples, name, spec->env.get());
        int example_num = std::min(40, int(examples.size())), share_num = 2;
        fta::FTAList base_list;
        auto* grammar = spec->info_list[0]->grammar;
        auto* env = spec->env.get();
        for (int i = 0; i < example_num; ++i) {
            auto fta = fta::buildFTA(grammar, env, examples[i], start_size, true);
            base_list.push_back(fta);
        }
        auto merged = base_list[0];
        std::vector<int> size_list = {merged->edgeCount()};
        std::cout << size_list[0] << std::endl;
        for (int i = 1; i < example_num; ++i) {
            merged = fta::mergeFTA(merged.get(), base_list[i].get(), fta::FORWARD, true);
            size_list.push_back(merged->edgeCount());
            std::cout << size_list[i] << std::endl;
        }
        std::cout << "[";
        for (int i = 0; i < size_list.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << size_list[i];
        }
        std::cout << "]" << std::endl;
        exit(0);
        auto shared = base_list[0];
        for (int i = 1; i < share_num; ++i) {
            shared = fta::mergeFTA(shared.get(), base_list[i].get(), fta::FORWARD, true);
        }
        fta::FTAList unit_list;
        for (int i = share_num; i < base_list.size(); ++i) {
            auto unit = fta::mergeFTA(shared.get(), base_list[i].get(), fta::FORWARD, true);
            unit_list.push_back(unit);
        }
        LOG(INFO) << "Forward start" << std::endl;
        auto forward = fta::merge::mergeFTAForwardMulti(unit_list, false);
        LOG(INFO) << "Forward  Raw " << fta::size::sizeInfo2String(fta::size::getNodeCountVector(forward.get()));
        forward->fullyCut();
        LOG(INFO) << "Forward  Fin " << fta::size::sizeInfo2String(fta::size::getNodeCountVector(forward.get()));
        /*auto backward = fta::merge::mergeFTABackwardMulti(base_list, false, share_num);
        LOG(INFO) << "Backward Raw " << fta::size::sizeInfo2String(fta::size::getNodeCountVector(backward.get()));
        backward->fullyCut();
        LOG(INFO) << "Backward Fin " << fta::size::sizeInfo2String(fta::size::getNodeCountVector(backward.get()));*/

        LOG(INFO) << "Backward start";
        auto bid2 = fta::merge::mergeFTAStagedBidirectional(unit_list, false, 0.5);
        LOG(INFO) << "Bid Raw " << fta::size::sizeInfo2String(fta::size::getNodeCountVector(bid2.get()));
        bid2->fullyCut();
        LOG(INFO) << "Bid Fin " << fta::size::sizeInfo2String(fta::size::getNodeCountVector(bid2.get()));

        //auto program = fta::util::extractMinimalProgram(backward.get());
        //LOG(INFO) << "Result " << program->toString();
        exit(0);
    }

    return 0;
    global::recorder.start("raw-cegis");
    auto cegis_program = fta::synthesis::rawCEGIS(spec, fta::FORWARD, start_size);
    global::recorder.end("raw-cegis");
    LOG(INFO) << "Raw Examples:";
    for (auto& example: global::example_recorder) {
        LOG(INFO) << "  " << example::ioExample2String(example);
    }

    global::example_recorder.clear();

    global::recorder.start("total");
    auto program = fta::synthesis::kFoldCEGIS(spec, fold_num, new fta::SizeExpectedScheduler(0.7, 3), fta::FORWARD, start_size);
    global::recorder.end("total");

    LOG(INFO) << "KFold Examples:";
    for (auto& example: global::example_recorder) {
        LOG(INFO) << "  " << example::ioExample2String(example);
    }

    global::recorder.start("raw-merge");
    auto fta = fta::synthesis::rawMerge(spec, global::example_recorder, fta::FORWARD, start_size);
    global::recorder.end("raw-merge");

    LOG(INFO) << "Program " << program->toString();
    LOG(INFO) << "RawMerge " << fta::util::extractMinimalProgram(fta.get())->toString();
    global::recorder.printAll();
}
