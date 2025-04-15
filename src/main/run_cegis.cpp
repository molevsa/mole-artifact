//
// Created by pro on 2024/9/18.
//

#include <random>
#include <unordered_set>

#include "glog/logging.h"
#include "istool/basic/config.h"
#include "istool/basic/verifier.h"
#include "istool/ext/z3/z3_example_space.h"
#include "istool/ext/z3/z3_verifier.h"
#include "istool/fta/fta_multi.h"
#include "istool/sygus/parser/parser.h"
#include "istool/sygus/theory/basic/clia/clia.h"

Verifier *_getVerifier(ExampleSpace *example_space) {
    auto *finite_io_space = dynamic_cast<FiniteExampleSpace *>(example_space);
    if (finite_io_space) {
        return new FiniteExampleVerifier(finite_io_space);
    }
    auto *z3_space = dynamic_cast<Z3ExampleSpace *>(example_space);
    if (z3_space) {
        return new Z3Verifier(z3_space);
    }
    LOG(FATAL) << "Unsupported example space";
}

IOExampleList generateExamples(Z3ExampleSpace *example_space, int example_num,
                               int int_range, Env *env) {
    bool is_all_bool = true;
    for (auto &inp_type : example_space->type_list) {
        if (!dynamic_cast<TBool *>(inp_type.get())) {
            is_all_bool = false;
        }
    }
    int n = example_space->type_list.size();
    ExampleList raw_examples;
    if (is_all_bool && n <= 7) {
        for (int state = 0; state < (1 << n); ++state) {
            DataList example(n);
            for (int i = 0; i < n; ++i) {
                if (state & (1 << i))
                    example[i] = BuildData(Bool, true);
                else
                    example[i] = BuildData(Bool, false);
            }
            raw_examples.push_back(example);
        }
    } else {
        auto gen = [=](Type *type) {
            if (dynamic_cast<TBool *>(type)) {
                return BuildData(Bool, rand() & 1);
            }
            if (dynamic_cast<TInt *>(type)) {
                return BuildData(Int, rand() % (2 * int_range + 1) - int_range);
            }
            LOG(FATAL) << "Unknown type " << type->getName();
        };
        std::unordered_set<std::string> known_examples;
        for (int _ = 0; _ < example_num; ++_) {
            DataList example(n);
            for (int i = 0; i < n; ++i)
                example[i] = gen(example_space->type_list[i].get());
            auto feature = data::dataList2String(example);
            if (known_examples.find(feature) != known_examples.end())
                continue;
            known_examples.insert(feature);
            raw_examples.push_back(example);
        }
    }

    std::shuffle(raw_examples.begin(), raw_examples.end(), env->random_engine);
    IOExampleList examples;
    auto *io_space = dynamic_cast<IOExampleSpace *>(example_space);
    assert(io_space);
    for (auto &example : raw_examples)
        examples.push_back(io_space->getIOExample(example));
    return examples;
}

const int KExampleNum = 1000;
const int config::KIntRange = 10;

void printResult(const std::vector<std::string> &info_list,
                 const std::string &path) {
    for (auto &info : info_list)
        std::cout << info << std::endl;
    if (!path.empty()) {
        auto f = std::fopen(path.c_str(), "w");
        for (auto &info : info_list) {
            fprintf(f, "%s\n", info.c_str());
        }
        fprintf(f, "%.5f\n", global::recorder.query("total"));
        fclose(f);
    }
}

int main(int argv, char **argc) {
    std::string task_path, res_path, exe_type;
    int start_size = 1;
    int init_num, shared_num;
    double alpha;
    if (argv > 1) {
        task_path = std::string(argc[1]);
        res_path = std::string(argc[2]);
        exe_type = std::string(argc[3]);
        if (argv == 7) {
            init_num = std::stoi(argc[4]);
            shared_num = std::stoi(argc[5]);
            alpha = std::stof(argc[6]);
        }
    } else {
        task_path =
            "/path/to/mole/benchmark/circuit/hd05.eqn_sygus_iter_120_0.sl";
        exe_type = "fold4";
        init_num = 15;
        shared_num = 1;
        alpha = 0.7;
    }
    config::KIsMultiThread = false;
    auto *spec = parser::getSyGuSSpecFromFile(task_path);
    spec->env->setRandomSeed(0);
    auto original_space = spec->example_space;
    auto *full_verifier = _getVerifier(spec->example_space.get());
    auto name = spec->info_list[0]->name;

    auto *z3_space = dynamic_cast<Z3ExampleSpace *>(spec->example_space.get());
    /*if (z3_space) {
        auto examples = generateExamples(z3_space, KExampleNum,
    config::KIntRange, spec->env.get()); spec->example_space =
    example::buildFiniteIOExampleSpace(examples, name, spec->env.get());
    }*/

    global::recorder.start("total");
    PProgram program;
    if (exe_type == "forward") {
        program = fta::synthesis::rawCEGIS(spec, fta::FORWARD, start_size);
    } else if (exe_type == "backward") {
        program = fta::synthesis::rawCEGIS(spec, fta::BACKWARD, start_size);
    } else if (exe_type.substr(0, 4) == "fold") {
        int fold_num = std::stoi(exe_type.substr(4));
        // program = fta::synthesis::kFoldCEGIS(spec, fold_num, new
        // fta::TimeAdaptiveScheduler(fold_num), start_size);
        program = fta::synthesis::kFoldCEGIS(
            spec, fold_num, new fta::SizeExpectedScheduler(0.7, 2),
            fta::FORWARD);
    } else if (exe_type.substr(0, 5) == "bfold") {
        int fold_num = std::stoi(exe_type.substr(5));
        // program = fta::synthesis::kFoldCEGIS(spec, fold_num, new
        // fta::TimeAdaptiveScheduler(fold_num), start_size);
        program = fta::synthesis::kFoldCEGIS(
            spec, fold_num, new fta::SizeExpectedScheduler(0.7, 2),
            fta::BACKWARD);
    } else if (exe_type == "bid") {
        fta::synthesis::BidConfig config(init_num, shared_num, alpha);
        program = fta::synthesis::bidCEGIS(spec, config);
    } else {
        LOG(FATAL) << "Unknown synthesis type";
    }
    global::recorder.end("total");

    if (program) {
        if (full_verifier->verify(semantics::buildSingleContext(name, program),
                                  nullptr)) {
            // TODO: use the same method to count total node and total edge
            printResult({program->toString(), std::to_string(program->size()),
                         std::to_string(global::node_count),
                         std::to_string(global::edge_count),
                         std::to_string(global::example_num)},
                        res_path);
        } else {
            printResult(
                {"Overfit " + std::to_string(KExampleNum) + " examples"},
                res_path);
        }
    } else {
        printResult({"No solution", std::to_string(global::node_count),
                     std::to_string(global::edge_count),
                     std::to_string(global::example_num)},
                    res_path);
    }
    global::recorder.printAll();
}