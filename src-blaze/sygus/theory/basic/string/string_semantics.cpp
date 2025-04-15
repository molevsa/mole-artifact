//
// Created by pro on 2021/12/28.
//

#include "istool/sygus/theory/basic/string/string_semantics.h"

#include <glog/logging.h>

#include <regex>

#include "istool/basic/env.h"
#include "istool/sygus/theory/basic/clia/clia.h"
#include "istool/sygus/theory/basic/string/str.h"

#define TSTRING theory::string::getTString()
#define TINT theory::clia::getTInt()
#define TBOOL type::getTBool()
using theory::clia::getIntValue;
using theory::string::getStringValue;

namespace {
    std::map<std::string, std::regex> pattern_map{
        {"ProperCase", std::regex("[A-Z][a-z]+")},
        {"CAPS", std::regex("[A-Z]+")},
        {"lowercase", std::regex("[a-z]+")},
        {"Digits", std::regex("[0-9]+")},
        {"Alphabets", std::regex("[a-zA-Z]+")},
        {"Alphanumeric", std::regex("[A-Za-z0-9]+")},
        {"WhiteSpace", std::regex("[ ]+")},
        {"StartT", std::regex("^")},
        {"EndT", std::regex("$")},
        {"ProperCaseWSpaces", std::regex("[A-Z][a-z]+( [A-Z][a-z]+)*")},
        {"CAPSWSpaces", std::regex("[A-Z]+( [A-Z]+)*")},
        {"lowercaseSpaces", std::regex("[a-z]+( [a-z]+)*")},
        {"AlphabetsWSpaces", std::regex("[a-zA-Z]+( [a-zA-Z]+)*")},
        {"Sep", std::regex("[|]")}};
}

StringCatSemantics::StringCatSemantics()
    : NormalSemantics("str.++", TSTRING, {TSTRING, TSTRING}) {}
Data StringCatSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    auto v0 = getStringValue(inp_list[0]), v1 = getStringValue(inp_list[1]);
    if (v0.sv_type == StringValueType::FALSE ||
        v1.sv_type == StringValueType::FALSE) {
        return BuildData(String, StringValueType::FALSE);
    }
    if (v0.sv_type != StringValueType::CONDITION) {
        return BuildData(String, StringValueType::TRUE);
    } else {
        StringValue res(StringValueType::CONDITION);
        if (v1.sv_type == StringValueType::CONDITION) {
            res.length = v0.length.value() + v1.length.value();
        } else {
            return BuildData(String, StringValueType::TRUE);
        }
        for (auto [ind, c] : v1.at) {
            res.at.insert(std::make_pair(v0.length.value() + ind, c));
        }
        for (auto [ind, c] : v0.at) {
            res.at.insert(std::make_pair(ind, c));
        }
        return BuildData(String, res);
    }
}

StringReplaceSemantics::StringReplaceSemantics()
    : NormalSemantics("str.replace", TSTRING, {TSTRING, TSTRING, TSTRING}) {}
Data StringReplaceSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    // TODO: more accurate semantics
    auto s = getStringValue(inp_list[0]), t1 = getStringValue(inp_list[1]),
         t2 = getStringValue(inp_list[2]);
    if (s.sv_type != StringValueType::CONDITION ||
        t1.sv_type != StringValueType::CONDITION) {
        return BuildData(String, StringValueType::TRUE);
    } else {
        int ls = s.length.value(), lt1 = t1.length.value();
        if (ls < lt1) {
            return BuildData(String, s);
        }
        // true case
        for (int i = 0; i + lt1 - 1 < ls; i++) {
            bool res = true;
            for (int j = 0; j < lt1; j++) {
                res &= (s.at.count(i + j) && t1.at.count(j) &&
                        s.at[i + j] == t1.at[j]);
            }
            if (res) {
                if (t2.sv_type == StringValueType::TRUE) {
                    return BuildData(String, StringValueType::TRUE);
                } else {
                    StringValue res(StringValueType::CONDITION);
                    int lt2 = t2.length.value();
                    res.length = ls - lt1 + lt2;
                    for (int k = 0; k < i; k++) {
                        if (s.at.count(k)) {
                            res.at[k] = s.at[k];
                        }
                    }
                    for (int k = 0; k < lt2; k++) {
                        if (t2.at.count(k)) {
                            res.at[i + k] = t2.at[k];
                        }
                    }
                    for (int k = i + lt1; k < ls; k++) {
                        if (s.at.count(k)) {
                            res.at[k - lt1 + lt2] = s.at[k];
                        }
                    }
                    return BuildData(String, res);
                }
            }
        }
        // maybe case
        int min_match_startpos = -1, max_match_startpos = -1;
        for (int i = 0; i + lt1 - 1 < ls; i++) {
            bool res = true;
            for (int j = 0; j < lt1; j++) {
                res &= (!s.at.count(i + j) || !t1.at.count(j) ||
                        s.at[i + j] == t1.at[j]);
            }
            if (res) {
                if (min_match_startpos == -1)
                    min_match_startpos = i;
                max_match_startpos = i;
            }
        }
        if (min_match_startpos != -1) {
            if (t2.sv_type == StringValueType::TRUE) {
                return BuildData(String, StringValueType::TRUE);
            } else if (t2.length.value() == lt1) {
                StringValue res(StringValueType::CONDITION);
                res.length = s.length;
                for (int k = 0; k < min_match_startpos; k++) {
                    if (s.at.count(k)) {
                        res.at[k] = s.at[k];
                    }
                }
                for (int k = max_match_startpos + lt1; k < ls; k++) {
                    if (s.at.count(k)) {
                        res.at[k] = s.at[k];
                    }
                }
                return BuildData(String, res);
            } else {
                return BuildData(String, StringValueType::TRUE);
            }
        }
        // false case
        return BuildData(String, s);
    }
}

StringAtSemantics::StringAtSemantics()
    : NormalSemantics("str.at", TSTRING, {TSTRING, TINT}) {}
Data StringAtSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    auto sv = getStringValue(inp_list[0]);
    auto id_op = getIntValue(inp_list[1]);
    if (!id_op.has_value()) {
        return BuildData(String, StringValueType::TRUE);
    } else if (sv.sv_type != StringValueType::CONDITION) {
        return BuildData(String, StringValueType::TRUE);
    } else {
        int id = id_op.value();
        int len = sv.length.value();
        if (id < 0 || id >= len) {
            return BuildData(String, "");
        } else {
            StringValue res(StringValueType::CONDITION);
            res.length = 1;
            if (sv.at.count(id))
                res.at[0] = sv.at[id];
            return BuildData(String, res);
        }
    }
}

StringSubStrSemantics::StringSubStrSemantics()
    : NormalSemantics("str.substr", TSTRING, {TSTRING, TINT, TINT}) {}
Data StringSubStrSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    std::string s = getStringValue(inp_list[0]).getFullString().value();
    if (!getIntValue(inp_list[1]).has_value() ||
        !getIntValue(inp_list[2]).has_value()) {
        return BuildData(String, StringValueType::FALSE);
    }
    int l = getIntValue(inp_list[1]).value();
    int r = getIntValue(inp_list[2]).value();
    if (l > r || l < 0 || r > s.length()) {
        return BuildData(String, StringValueType::FALSE);
    }
    return BuildData(String, s.substr(l, r - l));
}

StringContainsSemantics::StringContainsSemantics()
    : NormalSemantics("str.contains", TBOOL, {TSTRING, TSTRING}) {}
Data StringContainsSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    auto s = getStringValue(inp_list[0]), t = getStringValue(inp_list[1]);
    if (s.sv_type == StringValueType::TRUE ||
        t.sv_type == StringValueType::TRUE) {
        return BuildData(Bool, BoolValue());
    }
    int ls = s.length.value(), lt = t.length.value();
    if (ls < lt) {
        return BuildData(Bool, false);
    }
    // true case
    for (int i = 0; i + lt - 1 < ls; i++) {
        bool res = true;
        for (int j = 0; j < lt; j++) {
            res &=
                (s.at.count(i + j) && t.at.count(j) && s.at[i + j] == t.at[j]);
        }
        if (res)
            return BuildData(Bool, true);
    }
    // maybe case
    for (int i = 0; i + lt - 1 < ls; i++) {
        bool res = true;
        for (int j = 0; j < lt; j++) {
            res &= (!s.at.count(i + j) || !t.at.count(j) ||
                    s.at[i + j] == t.at[j]);
        }
        if (res)
            return BuildData(Bool, BoolValue());
    }
    // false case
    return BuildData(Bool, false);
}

StringPrefixOfSemantics::StringPrefixOfSemantics()
    : NormalSemantics("str.prefixof", TBOOL, {TSTRING, TSTRING}) {}
Data StringPrefixOfSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    auto s = getStringValue(inp_list[1]), t = getStringValue(inp_list[0]);
    if (s.sv_type == StringValueType::TRUE ||
        t.sv_type == StringValueType::TRUE) {
        return BuildData(Bool, BoolValue());
    }
    int ls = s.length.value(), lt = t.length.value();
    if (ls < lt) {
        return BuildData(Bool, false);
    }
    // true case
    bool res = true;
    for (int j = 0; j < lt; j++) {
        res &= (s.at.count(j) && t.at.count(j) && s.at[j] == t.at[j]);
    }
    if (res)
        return BuildData(Bool, true);
    // maybe case
    res = true;
    for (int j = 0; j < lt; j++) {
        res &= (!s.at.count(j) || !t.at.count(j) || s.at[j] == t.at[j]);
    }
    if (res)
        return BuildData(Bool, BoolValue());
    // false case
    return BuildData(Bool, false);
}

StringSuffixOfSemantics::StringSuffixOfSemantics()
    : NormalSemantics("str.suffixof", TBOOL, {TSTRING, TSTRING}) {}
Data StringSuffixOfSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    auto s = getStringValue(inp_list[1]), t = getStringValue(inp_list[0]);
    if (s.sv_type == StringValueType::TRUE ||
        t.sv_type == StringValueType::TRUE) {
        return BuildData(Bool, BoolValue());
    }
    int ls = s.length.value(), lt = t.length.value();
    if (ls < lt) {
        return BuildData(Bool, false);
    }
    int startpos = ls - lt;
    // true case
    bool res = true;
    for (int j = 0; j < lt; j++) {
        res &= (s.at.count(startpos + j) && t.at.count(j) &&
                s.at[startpos + j] == t.at[j]);
    }
    if (res)
        return BuildData(Bool, true);
    // maybe case
    res = true;
    for (int j = 0; j < lt; j++) {
        res &= (!s.at.count(startpos + j) || !t.at.count(j) ||
                s.at[startpos + j] == t.at[j]);
    }
    if (res)
        return BuildData(Bool, BoolValue());
    // false case
    return BuildData(Bool, false);
}

StringIndexOfSemantics::StringIndexOfSemantics()
    : NormalSemantics("str.indexof", TINT, {TSTRING, TSTRING, TINT, TINT}) {}
Data StringIndexOfSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    std::string s = getStringValue(inp_list[0]).getFullString().value();
    std::string pattern = getStringValue(inp_list[1]).getFullString().value();
    int pos = getIntValue(inp_list[2]).value();
    int dir = getIntValue(inp_list[3]).value();
    std::vector<std::pair<int, int>> match_result;
    match_result.emplace_back(-100, -100); // pos is 1-indexed
    if (pattern_map.count(pattern)) {
        auto reg = pattern_map[pattern];
        std::regex_iterator<std::string::const_iterator> begin(s.begin(),
                                                               s.end(), reg);
        for (auto iter = begin; iter != std::sregex_iterator(); iter++) {
            int l = iter->position();
            int len = iter->length();
            match_result.push_back(std::make_pair(l, l + len));
        }
    } else {
        for (auto l = s.find(pattern); l != std::string::npos;
             l = s.find(pattern, l + 1)) {
            match_result.push_back(std::make_pair(l, l + pattern.length() - 1));
        }
    }
    if (pos < 0) {
        pos = int(match_result.size()) + pos;
    }
    if (pos < 0 || pos >= match_result.size()) {
        return BuildData(Int, IntValue());
    }
    if (dir == 0)
        return BuildData(Int, match_result[pos].first);
    else
        return BuildData(Int, match_result[pos].second);
}

StringLenSemantics::StringLenSemantics()
    : NormalSemantics("str.len", TINT, {TSTRING}) {}
Data StringLenSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    auto sv = getStringValue(inp_list[0]);
    if (sv.sv_type != StringValueType::CONDITION) {
        return BuildData(Int, IntValue());
    } else {
        return BuildData(Int, sv.length.value());
    }
}

IntToStrSemantics::IntToStrSemantics()
    : NormalSemantics("int.to.str", TSTRING, {TINT}) {}
Data IntToStrSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    auto i = getIntValue(inp_list[0]);
    if (i.has_value()) {
        if (i < 0)
            return BuildData(String, "");
        return BuildData(String, std::to_string(i.value()));
    } else {
        return BuildData(String, StringValue(StringValueType::TRUE));
    }
}

StrToIntSemantics::StrToIntSemantics(Data *_inf)
    : inf(_inf), NormalSemantics("str.to.int", TINT, {TSTRING}) {}
Data StrToIntSemantics::run(DataList &&inp_list, ExecuteInfo *info) {
    auto sv = getStringValue(inp_list[0]);
    if (sv.sv_type == StringValueType::CONDITION) {
        bool num_nac = (sv.at.size() != sv.length.value());
        for (auto [ind, c] : sv.at) {
            if (c > '9' || c < '0')
                return BuildData(Int, -1);
        }
        if (num_nac)
            return BuildData(Int, IntValue());
        int res = 0;
        for (auto [ind, c] : sv.at) {
            long long ne = 10ll * res + int(c - '0');
            if (ne > getIntValue(*inf))
                throw SemanticsError();
            res = ne;
        }
        return BuildData(Int, res);
    } else {
        return BuildData(Int, IntValue());
    }
}
