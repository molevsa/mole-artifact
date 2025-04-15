#include <glog/logging.h>

#include <algorithm>

#include "istool/fta/fta_value.h"

namespace fta::value {
    FTAValueSet::FTAValueSet() : string_value_sets() {}
    Data FTAValueSet::getAvailableValue(const Data &data) {
        auto v = dynamic_cast<StringValue *>(data.get());
        if (!v) {
            return data;
        }
        if (v->sv_type != StringValueType::CONDITION) {
            return data;
        }
        for (auto it = string_value_sets[v->length.value()].rbegin();
             it != string_value_sets[v->length.value()].rend(); it++) {
            if ((*it).isSubValue(v)) {
                return BuildData(String, *it);
            }
        }
        StringValue res(StringValueType::CONDITION);
        res.length = v->length.value();
        return BuildData(String, res);
    }
    void FTAValueSet::addValue(const Data &data) {
        auto v = dynamic_cast<StringValue *>(data.get());
        if (!v) {
            // TODO: support abstract int and bool value
            return;
        }
        if (v->sv_type == StringValueType::TRUE) {
            return;
        }
        assert(v->length.has_value());
        string_value_sets[v->length.value()].insert(*v);
    }

    int FTAValueSet::size() {
        int res = 0;
        for (auto [_, s] : string_value_sets) {
            res += s.size();
        }
        return res;
    }
    FTAValueSets::FTAValueSets() : examples(), value_sets() {}
    void FTAValueSets::addValue(const IOExample &example, const Data &data) {
        auto example_index =
            std::find(examples.begin(), examples.end(), example) -
            examples.begin();
        if (example_index >= examples.size()) {
            examples.push_back(example);
            value_sets.emplace_back();
        }
        value_sets[example_index].addValue(data);
    }
    void FTAValueSets::initializeValueSet(const IOExample &example) {
        if (std::count(examples.begin(), examples.end(), example))
            return;
        int example_index = examples.size();
        examples.push_back(example);
        value_sets.emplace_back();
        for (auto in : example.first) {
            value_sets[example_index].addValue(in);
        }
        // add regex id
        value_sets[example_index].addValue(BuildData(String, "ProperCase"));
        value_sets[example_index].addValue(BuildData(String, "CAPS"));
        value_sets[example_index].addValue(BuildData(String, "lowercase"));
        value_sets[example_index].addValue(BuildData(String, "Digits"));
        value_sets[example_index].addValue(BuildData(String, "Alphabets"));
        value_sets[example_index].addValue(BuildData(String, "Alphanumeric"));
        value_sets[example_index].addValue(BuildData(String, "WhiteSpace"));
        value_sets[example_index].addValue(BuildData(String, "StartT"));
        value_sets[example_index].addValue(BuildData(String, "EndT"));
        value_sets[example_index].addValue(
            BuildData(String, "ProperCaseWSpaces"));
        value_sets[example_index].addValue(BuildData(String, "CAPSWSpaces"));
        value_sets[example_index].addValue(
            BuildData(String, "lowercaseSpaces"));
        value_sets[example_index].addValue(
            BuildData(String, "AlphabetsWSpaces"));
        value_sets[example_index].addValue(BuildData(String, "Sep"));
    }
    Data FTAValueSets::getAvailableValue(const IOExample &example,
                                         const Data &data) {
        auto example_index =
            std::find(examples.begin(), examples.end(), example) -
            examples.begin();
        if (example_index >= examples.size()) {
            examples.emplace_back(example);
            value_sets.emplace_back();
        }
        return value_sets[example_index].getAvailableValue(data);
    }
    int FTAValueSets::size() {
        int res = 0;
        for (auto s : value_sets)
            res += s.size();
        return res;
    }
    bool isSubValue(const Data &x, const Data &y) {
        {
            // StringValue
            auto vx = dynamic_cast<StringValue *>(x.get()),
                 vy = dynamic_cast<StringValue *>(y.get());
            if (vx && vy)
                return vx->isSubValue(vy);
        }
        {
            // BoolValue
            auto vx = dynamic_cast<BoolValue *>(x.get()),
                 vy = dynamic_cast<BoolValue *>(y.get());
            if (vx && vy)
                return vx->isSubValue(vy);
        }
        {
            // IntValue
            auto vx = dynamic_cast<IntValue *>(x.get()),
                 vy = dynamic_cast<IntValue *>(y.get());
            if (vx && vy)
                return vx->isSubValue(vy);
        }
        return x == y;
    }
    Data execByValue(Rule *rule, DataList param_list, Env *env,
                     DataList input) {
        if (!rule->param_list.empty()) {
            auto semantics = grammar::getFullExecutedSemantics(rule);
            assert(semantics);
            return semantics->run(std::move(param_list), nullptr);
        } else {
            ProgramList sub_list;
            auto prog = rule->buildProgram(sub_list);
            return env->run(prog.get(), input);
        }
    }
    Data getAvailableValue(FTAValueSets *value_sets, const IOExample &example,
                           const Data &data) {
        return value_sets->getAvailableValue(example, data);
    }
} // namespace fta::value
