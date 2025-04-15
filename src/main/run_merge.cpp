//
// Created by pro on 2024/9/18.
//

#include "istool/fta/fta_multi.h"
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

std::mt19937 rng(0);
IOExampleList generateExamples(Z3ExampleSpace* example_space, int example_num, int int_range, fta::FTA* program_space) {
    ExampleList raw_examples;
    auto* verifier = new Z3Verifier(example_space);
    auto func_name = example_space->sig_map.begin()->first;
    auto& ctx = example_space->ext->ctx;
    auto* env = example_space->env;
    for (int _ = 0; _ < example_num; ++_) {
        auto random_program = fta::util::extractRandomMinimalProgram(program_space, example_space->env);
        z3::solver solver(ctx);
        verifier->prepareZ3Solver(solver, semantics::buildSingleContext(func_name, random_program));
        auto res = solver.check();
        if (res != z3::sat) continue;
        auto model = solver.get_model(); Example raw_example;
        verifier->getExample(model, &raw_example);
        for (int i = 0; i < example_space->type_list.size(); ++i) {
            auto type = example_space->type_list[i];
            auto var = example_space->ext->buildVar(type.get(), "Param" + std::to_string(i));
            if (dynamic_cast<TBool*>(type.get())) {
                auto dist = std::bernoulli_distribution(0.5);
                auto expected = dist(env->random_engine);
                solver.push();
                solver.add(var == ctx.bool_val(expected));
                auto current_res = solver.check();
                if (current_res != z3::sat) solver.pop();
            }
            if (dynamic_cast<TInt*>(type.get())) {
                auto int_dist = std::uniform_int_distribution<int>(-int_range, int_range);
                auto expected = int_dist(env->random_engine);
                solver.push();
                solver.add(var == ctx.int_val(expected));
                auto current_res = solver.check();
                if (current_res == z3::sat) continue; else solver.pop();
                auto bool_dist = std::bernoulli_distribution(0.5);
                auto is_change = bool_dist(env->random_engine);
                if (is_change) {
                    solver.push();
                    solver.add(var != example_space->ext->buildConst(raw_example[i]));
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
    for (auto& example: raw_examples) examples.push_back(io_space->getIOExample(example));
    return examples;
}

const int config::KIntRange = 10;

IOExampleList _getExamples(Specification* spec, int example_num, int size_limit) {
    auto* fio = dynamic_cast<FiniteIOExampleSpace*>(spec->example_space.get());
    if (fio) {
        IOExampleList result;
        for (auto& example: fio->example_space) {
            result.push_back(fio->getIOExample(example));
        }
        if (result.size() > example_num) result.resize(example_num);
        return result;
    }
    auto* zio = dynamic_cast<Z3IOExampleSpace*>(spec->example_space.get());
    if (zio) {
        auto fta = fta::grammar2FTA(spec->info_list[0]->grammar, size_limit, true);
        IOExampleList result = generateExamples(zio, example_num << 1, config::KIntRange, fta.get());
        if (result.size() > example_num) result.resize(example_num);
        return result;
    }
    LOG(FATAL) << "Unsupported example space";
}

void printResult(const std::vector<std::string>& info_list, const std::string& path) {
    for (auto& info: info_list) std::cout << info << std::endl;
    if (!path.empty()) {
        auto f = std::fopen(path.c_str(), "w");
        for (auto& info: info_list) {
            fprintf(f, "%s\n", info.c_str());
        }
        fprintf(f, "%d\n", global::node_count);
        fprintf(f, "%d\n", global::edge_count);
        fprintf(f, "%.5f\n", global::recorder.query("multi"));
        fclose(f);
    }
}

int main(int argv, char** argc) {
    std::string task_path, res_path, exe_type;
    int example_num, size;
    int init_num, shared_num;
    double alpha;
    if (argv > 1) {
        assert(argv >= 6);
        task_path = std::string(argc[1]);
        res_path = std::string(argc[2]);
        exe_type = std::string(argc[3]);
        size = std::atoi(argc[4]);
        example_num = std::stoi(argc[5]);
        if (argv == 9) {
            init_num = std::atoi(argc[6]);
            shared_num = std::stoi(argc[7]);
            alpha = std::stof(argc[8]);
        }
    } else {
        task_path =
            "/path/to/mole/benchmark/circuit/hd05.eqn_sygus_iter_120_0.sl";
        exe_type = "fold4";
        example_num = 40;
        size = 27;
        init_num = 10;
        shared_num = 1;
        alpha = 0.6;
    }
    config::KIsMultiThread = false;

    auto* spec = parser::getSyGuSSpecFromFile(task_path);
    spec->env->setRandomSeed(0);
    auto* full_verifier = _getVerifier(spec->example_space.get());
    auto name = spec->info_list[0]->name;
    IOExampleList examples = _getExamples(spec, example_num, size);

    //global::recorder.start("total");
    fta::PFTA fta;
    if (exe_type == "forward") {
        fta = fta::synthesis::rawMerge(spec, examples, fta::FORWARD, size);
    } else if (exe_type == "backward") {
        fta = fta::synthesis::rawMerge(spec, examples, fta::BACKWARD, size);
    } else if (exe_type.substr(0, 6) == "foldnm") {
        int fold_num = std::stoi(exe_type.substr(6, 1));
        int example_num = std::stoi(exe_type.substr(7, 1));
        fta = fta::synthesis::kFoldMerge(spec, examples, fold_num, new fta::SizeExpectedScheduler(0, example_num), fta::FORWARD, size);
    } else if (exe_type.substr(0, 5) == "foldf") {
        int fold_num = std::stoi(exe_type.substr(5));
        fta = fta::synthesis::kFoldMerge(spec, examples, fold_num, new fta::SizeExpectedScheduler(0, 2), fta::FORWARD, size);
    } else if (exe_type.substr(0, 5) == "foldb") {
        int fold_num = std::stoi(exe_type.substr(5));
        fta = fta::synthesis::kFoldMerge(spec, examples, fold_num, new fta::SizeExpectedScheduler(0, 2), fta::BACKWARD, size);
    } else if (exe_type == "bid") {
        fta::synthesis::BidConfig config(init_num, shared_num, alpha);
        fta = fta::synthesis::bidMerge(spec, examples, config, size);
    } else{
        LOG(FATAL) << "Unknown execute type " << exe_type;
    }
    //global::recorder.end("total");

    if (!fta::util::isEmpty(fta.get())) {
        auto program = fta::util::extractMinimalProgram(fta.get());
        if (full_verifier->verify(semantics::buildSingleContext(name, program), nullptr)) {
            printResult({program->toString()}, res_path);
        } else {
            printResult({"Overfit: " + program->toString()}, res_path);
        }
    } else {
        printResult({"No solution"}, res_path);
    }
    global::recorder.printAll();
}