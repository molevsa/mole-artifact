#ifndef ISTOOL_FTA_VALUE_H
#define ISTOOL_FTA_VALUE_H

#include <cassert>
#include <map>
#include <set>

#include "istool/basic/data.h"
#include "istool/basic/env.h"
#include "istool/basic/example_space.h"
#include "istool/basic/grammar.h"
#include "istool/sygus/theory/basic/clia/clia_value.h"
#include "istool/sygus/theory/basic/string/string_value.h"

namespace fta::value {
    class FTAValueSet {
        std::map<int, std::set<StringValue>>
            string_value_sets; // All available StringValue in the FTA.
    public:
        FTAValueSet();
        Data getAvailableValue(const Data &data);
        void addValue(const Data &data);
        int size();
    };
    class FTAValueSets {
        std::vector<IOExample> examples;
        std::vector<FTAValueSet> value_sets;

    public:
        FTAValueSets();
        Data getAvailableValue(const IOExample &example, const Data &data);
        void addValue(const IOExample &example, const Data &data);
        void initializeValueSet(const IOExample &example);
        int size();
    };
    bool isSubValue(const Data &x, const Data &y);
    Data execByValue(Rule *rule, DataList param_list, Env *env, DataList input);
    Data getAvailableValue(const IOExample &example, const Data &data);
} // namespace fta::value

#endif