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
#include "istool/obe/obe.h"
#include "istool/sygus/parser/parser.h"
#include "istool/sygus/sygus.h"
#include "istool/sygus/theory/basic/clia/clia.h"

Verifier* _getVerifier(ExampleSpace* example_space) {
    auto* finite_io_space = dynamic_cast<FiniteExampleSpace*>(example_space);
    if (finite_io_space) {
        return new FiniteExampleVerifier(finite_io_space);
    }
    auto* z3_space = dynamic_cast<Z3ExampleSpace*>(example_space);
    if (z3_space) {
        return new Z3Verifier(z3_space);
    }
    LOG(FATAL) << "Unsupported example space";
}

std::mt19937 rng(0);
IOExampleList generateExamples(Z3ExampleSpace* example_space, int example_num,
                               int int_range, fta::FTA* program_space) {
    ExampleList raw_examples;
    auto* verifier = new Z3Verifier(example_space);
    auto func_name = example_space->sig_map.begin()->first;
    auto& ctx = example_space->ext->ctx;
    auto* env = example_space->env;
    for (int _ = 0; _ < example_num; ++_) {
        auto random_program = fta::util::extractRandomMinimalProgram(
            program_space, example_space->env);
        z3::solver solver(ctx);
        verifier->prepareZ3Solver(
            solver, semantics::buildSingleContext(func_name, random_program));
        auto res = solver.check();
        if (res != z3::sat) {
            --_;
            continue;
        }
        auto model = solver.get_model();
        Example raw_example;
        verifier->getExample(model, &raw_example);
        for (int i = 0; i < example_space->type_list.size(); ++i) {
            auto type = example_space->type_list[i];
            auto var = example_space->ext->buildVar(
                type.get(), "Param" + std::to_string(i));
            if (dynamic_cast<TBool*>(type.get())) {
                auto dist = std::bernoulli_distribution(0.5);
                auto expected = dist(env->random_engine);
                solver.push();
                solver.add(var == ctx.bool_val(expected));
                auto current_res = solver.check();
                if (current_res != z3::sat) solver.pop();
            }
            if (dynamic_cast<TInt*>(type.get())) {
                auto int_dist =
                    std::uniform_int_distribution<int>(-int_range, int_range);
                auto expected = int_dist(env->random_engine);
                solver.push();
                solver.add(var == ctx.int_val(expected));
                auto current_res = solver.check();
                if (current_res == z3::sat)
                    continue;
                else
                    solver.pop();
                auto bool_dist = std::bernoulli_distribution(0.5);
                auto is_change = bool_dist(env->random_engine);
                if (is_change) {
                    solver.push();
                    solver.add(var !=
                               example_space->ext->buildConst(raw_example[i]));
                    current_res = solver.check();
                    if (current_res != z3::sat) solver.pop();
                }
            }
        }
        assert(solver.check() == z3::sat);
        model = solver.get_model();
        verifier->getExample(model, &raw_example);
        raw_examples.push_back(raw_example);
    }

    std::shuffle(raw_examples.begin(), raw_examples.end(), rng);
    IOExampleList examples;
    auto* io_space = dynamic_cast<IOExampleSpace*>(example_space);
    assert(io_space);
    for (auto& example : raw_examples) {
        auto io_example = io_space->getIOExample(example);
        LOG(INFO) << example::ioExample2String(io_example);
        examples.push_back(io_example);
    }
    return examples;
}

const int KExampleNum = 100;
const int config::KIntRange = 10;

void printResult(const std::vector<std::string>& info_list,
                 const std::string& path) {
    for (auto& info : info_list) std::cout << info << std::endl;
    if (!path.empty()) {
        auto f = std::fopen(path.c_str(), "w");
        for (auto& info : info_list) {
            fprintf(f, "%s\n", info.c_str());
        }
        // fprintf(f, "%.5f\n", global::recorder.query("total"));
        // fprintf(f, "%d\n", global::node_count);
        // fprintf(f, "%d\n", global::edge_count);
        // fprintf(f, "%d\n", global::example_num);
        fclose(f);
    }
}

void fillExamples(Specification* old_spec, IOExampleList& example_list,
                  PExampleSpace example_space, int target_cnt, Env* env,
                  int prog_size) {
    if (target_cnt <= example_list.size()) return;
    auto z3_space = dynamic_cast<Z3ExampleSpace*>(example_space.get());
    if (z3_space) {
        auto program_space =
            fta::grammar2FTA(old_spec->info_list[0]->grammar, prog_size, false);
        auto new_examples =
            generateExamples(z3_space, target_cnt - example_list.size(),
                             config::KIntRange, program_space.get());
        for (auto example : new_examples) example_list.push_back(example);
        return;

    } else {
        auto finite_space =
            dynamic_cast<FiniteExampleSpace*>(example_space.get());
        auto io_space = dynamic_cast<IOExampleSpace*>(example_space.get());
        assert(finite_space);
        auto finite_examples = finite_space->example_space;
        std::unordered_set<std::string> known_examples;
        for (auto example : example_list) {
            auto feature = data::dataList2String(example.first);
            known_examples.insert(feature);
        }
        for (auto example : finite_examples) {
            if (example_list.size() >= target_cnt) return;
            auto new_feature = data::dataList2String(example);
            if (known_examples.count(new_feature)) continue;
            example_list.push_back(io_space->getIOExample(example));
        }
    }
    auto name = old_spec->info_list[0]->name;
    old_spec->example_space =
        example::buildFiniteIOExampleSpace(example_list, name, env);
}

int main(int argv, char** argc) {
    std::string task_path, res_path, exe_type, orig_task_path;
    const int KTargetExampleNum = 15;
    int start_size = 1;
    int fold_example_cnt = -1;
    int example_needed = 0;
    int init_num, shared_num;
    int fwd_node_cnt, best_node_cnt;
    int prog_size = -1;
    bool can_build_example = false;
    double alpha = 0.5;
    if (argv > 1) {
        task_path = std::string(argc[1]);
        res_path = std::string(argc[2]);
        exe_type = std::string(argc[3]);
        prog_size = std::stoi(argc[4]);
        /*if (argv == 7) {
            init_num = std::stoi(argc[4]);
            shared_num = std::stoi(argc[5]);
            alpha = std::stof(argc[6]);
        }*/
        /*if (argv == 5) {
            fold_example_cnt = std::stoi(argc[4]);
        }*/
    } else {
        task_path =
            "/home/jiry/2024A/duet/tests/circuit/hd05.eqn_sygus_iter_120_0.sl";
        // task_path = config::KSourcePath + "/tests/max3.sl";
        exe_type = "fold4";
        init_num = 15;
        shared_num = 1;
        alpha = 0.7;
    }
    // config::KIsMultiThread = false;
    auto* spec = parser::getSyGuSSpecFromFile(task_path);

    auto* env = spec->env.get();
    spec->env->setRandomSeed(0);
    auto example_space = spec->example_space;
    auto* full_verifier = _getVerifier(spec->example_space.get());
    auto name = spec->info_list[0]->name;

    auto grammar = spec->info_list[0]->grammar;
    auto max_size = grammar::getMaxSize(grammar);
    if (max_size != -1 && max_size < prog_size) prog_size = max_size;
    IOExampleList examples;
    auto z3_space = dynamic_cast<Z3ExampleSpace*>(example_space.get());
    if (z3_space) {
        auto prog_space = fta::grammar2FTA(grammar, prog_size, true);
        examples = generateExamples(z3_space, KExampleNum, config::KIntRange,
                                    prog_space.get());
        auto finite_space =
            example::buildFiniteIOExampleSpace(examples, name, env);
        spec->example_space = finite_space;
    } else {
        auto finite_space =
            dynamic_cast<FiniteExampleSpace*>(example_space.get());
        if (finite_space) {
            auto io_space = dynamic_cast<IOExampleSpace*>(example_space.get());
            auto finite_example_space = finite_space->example_space;
            for (auto example : finite_example_space) {
                auto io_example = io_space->getIOExample(example);
                LOG(INFO) << example::ioExample2String(io_example);
                examples.push_back(io_example);
            }
        }
    }
    spec->env->setRandomSeed(0);

    LOG(INFO) << grammar::getMaxSize(spec->info_list[0]->grammar);

    example_needed = examples.size();
    double min_time = 9999.0;
    int min_time_idx = -1;
    global::recorder.start("total");
    PProgram program;
    std::string program_str;  // only for obe

    if (exe_type == "forward") {
        global::recorder.start("fold_time");
        double start_time = global::recorder.query("fold_time");
        spec->env->setRandomSeed(0);
        auto fta =
            fta::synthesis::rawMerge(spec, examples, fta::FORWARD, prog_size);
        program = fta::util::extractMinimalProgram(fta.get());
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
        best_node_cnt = global::node_count;
    } else if (exe_type == "backward") {
        global::recorder.start("fold_time");
        double start_time = global::recorder.query("fold_time");
        spec->env->setRandomSeed(0);
        auto fta =
            fta::synthesis::rawMerge(spec, examples, fta::BACKWARD, prog_size);
        program = fta::util::extractMinimalProgram(fta.get());
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
        best_node_cnt = global::node_count;
    } else if (exe_type == "fwdfast") {
        global::recorder.start("fold_time");
        double start_time = global::recorder.query("fold_time");
        spec->env->setRandomSeed(0);
        auto fta = fta::synthesis::rawMergeFast(spec, examples, fta::FORWARD,
                                                prog_size);
        program = fta::util::extractMinimalProgram(fta.get());
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
        best_node_cnt = global::node_count;
    } else if (exe_type == "foldall") {
        // fillExamples(spec, examples, orig_example_space, KTargetExampleNum,
        // env, prog_size);
        LOG(INFO) << examples.size();
        global::recorder.start("fold_time");
        spec->env->setRandomSeed(0);
        double start_time = global::recorder.query("fold_time");
        auto fta =
            fta::synthesis::allMerge(spec, examples, fta::FORWARD, prog_size);
        program = fta::util::extractMinimalProgram(fta.get());
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
        best_node_cnt = global::node_count;
    } else if (exe_type.substr(0, 4) == "ffld") {
        // fillExamples(spec, examples, orig_example_space, KTargetExampleNum,
        // env, prog_size);
        LOG(INFO) << examples.size();
        // LOG(INFO) << "EXAMPLE CNT " << global::example_recorder.size();
        int fold_num = std::stoi(exe_type.substr(4));
        // program = fta::synthesis::kFoldCEGIS(spec, fold_num, new
        // fta::TimeAdaptiveScheduler(fold_num), start_size);
        global::recorder.start("fold_time");
        spec->env->setRandomSeed(0);
        double start_time = global::recorder.query("fold_time");
        auto fta = fta::synthesis::multiMerge(spec, examples, fold_num,
                                              fta::FORWARD, prog_size);
        program = fta::util::extractMinimalProgram(fta.get());
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
        best_node_cnt = global::node_count;
    } else if (exe_type.substr(0, 4) == "fold") {
        int fold_num = std::stoi(exe_type.substr(4));
        // program = fta::synthesis::kFoldCEGIS(spec, fold_num, new
        // fta::TimeAdaptiveScheduler(fold_num), start_size);

        global::recorder.start("fold_time");
        spec->env->setRandomSeed(0);
        double start_time = global::recorder.query("fold_time");
        auto fta = fta::synthesis::kFoldMerge(
            spec, examples, fold_num, new fta::SizeExpectedScheduler(alpha),
            fta::FORWARD, prog_size);
        program = fta::util::extractMinimalProgram(fta.get());
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
        best_node_cnt = global::node_count;
    } else if (exe_type.substr(0, 5) == "bfold") {
        int fold_num = std::stoi(exe_type.substr(5));
        // program = fta::synthesis::kFoldCEGIS(spec, fold_num, new
        // fta::TimeAdaptiveScheduler(fold_num), start_size);

        global::recorder.start("fold_time");
        spec->env->setRandomSeed(0);
        double start_time = global::recorder.query("fold_time");
        auto fta = fta::synthesis::kFoldMerge(
            spec, examples, fold_num, new fta::SizeExpectedScheduler(alpha),
            fta::BACKWARD, prog_size);
        program = fta::util::extractMinimalProgram(fta.get());
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
        best_node_cnt = global::node_count;
    } else if (exe_type.substr(0, 5) == "cfold") {
        int fold_num = std::stoi(exe_type.substr(5));
        // program = fta::synthesis::kFoldCEGIS(spec, fold_num, new
        // fta::TimeAdaptiveScheduler(fold_num), start_size);

        global::recorder.start("fold_time");
        spec->env->setRandomSeed(0);
        double start_time = global::recorder.query("fold_time");
        auto fta = fta::synthesis::kFoldMergeCart(
            spec, examples, fold_num, new fta::SizeExpectedScheduler(alpha),
            fta::FORWARD, prog_size);
        program = fta::util::extractMinimalProgram(fta.get());
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
        best_node_cnt = global::node_count;
    } else if (exe_type == "obe") {
        auto verifier = sygus::getVerifier(spec);
        assert(dynamic_cast<FiniteExampleVerifier*>(verifier));
        global::recorder.start("fold_time");
        double start_time = global::recorder.query("fold_time");
        program_str = obe::synthesis(spec, verifier, examples);
        global::recorder.end("fold_time");
        double end_time = global::recorder.query("fold_time");
        min_time = end_time - start_time;
    } else {
        LOG(FATAL) << "Unknown synthesis type";
        assert(0);
    }
    global::recorder.end("total");

    if (program) {
        printResult({program->toString(), std::to_string(min_time),
                     std::to_string(global::node_count),
                     std::to_string(global::edge_count),
                     std::to_string(example_needed)},
                    res_path);
    } else if (exe_type == "obe") {
        printResult({program_str, std::to_string(min_time),
                     std::to_string(global::node_count),
                     std::to_string(global::edge_count),
                     std::to_string(example_needed)},
                    res_path);
    } else {
        printResult({"No solution", std::to_string(min_time),
                     std::to_string(global::node_count),
                     std::to_string(global::edge_count),
                     std::to_string(example_needed)},
                    res_path);
    }
    global::recorder.printAll();
}