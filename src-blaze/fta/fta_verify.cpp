//
// Created by pro on 2024/10/6.
//

#include "istool/fta/fta_multi.h"
#include "istool/ext/z3/z3_verifier.h"
#include "istool/ext/z3/z3_example_space.h"
#include <unordered_set>

using namespace fta;

bool verify::verifyAfterExcludingExamples(const PProgram &program, Specification *spec, IOExample &counter_example,
                                          const IOExampleList &used_examples) {
    std::unordered_set<std::string> exclude_set;
    for (auto& example: used_examples) exclude_set.insert(example::ioExample2String(example));

    auto* finite_io_space = dynamic_cast<FiniteIOExampleSpace*>(spec->example_space.get());
    if (finite_io_space) {
        // LOG(INFO) << "Current program " << program->toString();
        for (auto& example: finite_io_space->example_space) {
            auto io_example = finite_io_space->getIOExample(example);
            if (example::satisfyIOExample(program.get(), io_example, spec->env.get())) {
                continue;
            }
            if (exclude_set.find(example::ioExample2String(io_example)) != exclude_set.end()) {
                continue;
            }
            counter_example = io_example;
            return false;
        }
        return true;
    }
    auto* z3_io_space = dynamic_cast<Z3IOExampleSpace*>(spec->example_space.get());
    if (z3_io_space) {
        auto& ctx = z3_io_space->ext->ctx;
        z3::solver solver(ctx);
        auto verifier = std::make_shared<Z3Verifier>(z3_io_space);

        auto func_name = spec->info_list[0]->name;
        auto res_info = semantics::buildSingleContext(func_name, program);
        verifier->prepareZ3Solver(solver, res_info);
        auto* ext = z3_io_space->ext;

        while (solver.check() == z3::sat) {
            Example example;
            auto model = solver.get_model();
            verifier->getExample(model, &example);
            counter_example = z3_io_space->getIOExample(example);
            if (exclude_set.find(example::ioExample2String(counter_example)) == exclude_set.end()) {
                return false;
            }

            z3::expr_vector diff_cons(ext->ctx);
            for (int i = 0; i < z3_io_space->type_list.size(); ++i) {
                auto type = z3_io_space->type_list[i];
                auto var = ext->buildVar(type.get(), "Param" + std::to_string(i));
                auto val = ext->buildConst(example[i]);
                diff_cons.push_back(var != val);
            }
            solver.add(z3::mk_or(diff_cons));
        }
        return true;
    }
    LOG(FATAL) << "Unknown example space";
}