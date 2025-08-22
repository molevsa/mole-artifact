#include "istool/obe/obe.h"

#include "istool/invoker/invoker.h"

std::string obe::synthesis(Specification* spec, Verifier* verifier,
                           IOExampleList io_examples) {
    InvokeConfig config;
    ExampleList examples;
    for (auto io_example : io_examples) {
        Example example = io_example.first;
        example.push_back(io_example.second);
        examples.push_back(example);
    }
    auto* solver = invoker::single::buildOBE(spec, verifier, examples, config);
    auto res = solver->synthesis();
    delete solver;
    return res.toString();
}