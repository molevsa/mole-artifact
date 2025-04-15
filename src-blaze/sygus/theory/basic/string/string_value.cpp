//
// Created by pro on 2021/12/28.
//

#include "istool/sygus/theory/basic/string/string_value.h"

#include "glog/logging.h"
#include "istool/sygus/theory/basic/string/string_type.h"

StringValue::StringValue(std::string str)
    : sv_type(StringValueType::CONDITION), Value() {
    length = str.length();
    int ind = 0;
    for (auto c : str) {
        at.insert(std::make_pair(ind, c));
        ind++;
    }
}
StringValue::StringValue(StringValue *value)
    : sv_type(value->sv_type), length(value->length), at(value->at), Value() {}
StringValue::StringValue(StringValueType sv_type) : sv_type(sv_type), Value() {}
std::string StringValue::toString() const {
    if (isFullString()) {
        return getFullString().value();
    } else if (sv_type == StringValueType::TRUE) {
        return "[TRUE String Value]";
    } else if (sv_type == StringValueType::FALSE) {
        return "[FALSE String Value]";
    } else {
        std::string res = "[CONDITION String Value, length = " +
                          std::to_string(length.value()) +
                          ", index condition: ";
        for (auto [ind, c] : at) {
            res += "ind = " + std::to_string(ind) + " c = " + c;
        }
        res += "]";
        return res;
    }
}
bool StringValue::operator<(const StringValue &v) const {
    if (sv_type == StringValueType::TRUE)
        return v.sv_type != StringValueType::TRUE;
    else {
        if (v.sv_type == StringValueType::TRUE)
            return false;
        else if (length.value() != v.length.value())
            return length.value() < v.length.value();
        else
            return (at.size() != v.at.size() ? at.size() < v.at.size()
                                             : at < v.at);
    }
}
bool StringValue::equal(Value *value) const {
    auto *sv = dynamic_cast<StringValue *>(value);
    if (!sv) {
        LOG(FATAL) << "Expect StringValue, but get " << value->toString();
    }
    if (sv_type != sv->sv_type)
        return false;
    if (sv_type == StringValueType::TRUE)
        return true;
    return length == sv->length && at == sv->at;
}
bool StringValue::isFullString() const {
    if (length) {
        for (int i = 0; i < length; i++) {
            if (!at.count(i))
                return false;
        }
        return true;
    } else {
        return false;
    }
}
std::optional<std::string> StringValue::getFullString() const {
    if (!isFullString())
        return std::optional<std::string>();
    std::string res;
    for (auto [ind, c] : at) {
        res.push_back(c);
    }
    return res;
}
bool StringValue::isSubValue(Value *value) const {
    auto v = dynamic_cast<StringValue *>(value);
    if (!v) {
        return false;
    }
    if (sv_type == StringValueType::TRUE)
        return true;
    if (sv_type == StringValueType::FALSE)
        return false;
    if (v->sv_type == StringValueType::TRUE)
        return false;
    if (v->sv_type == StringValueType::FALSE)
        return true;
    if (length != v->length)
        return false;
    for (auto [ind, c] : at) {
        if (!v->at.count(ind) || v->at.at(ind) != c)
            return false;
    }
    return true;
}
StringValue theory::string::getStringValue(const Data &d) {
    auto *sv = dynamic_cast<StringValue *>(d.get());
    if (!sv) {
        LOG(FATAL) << "Expect StringValue, but get " << d.toString();
    }
    if (sv->sv_type == StringValueType::CONDITION && !sv->length.has_value()) {
        LOG(FATAL)
            << "Condition StringValue with no length info is not allowed!";
    }
    return *sv;
}

StringValueTypeInfo::StringValueTypeInfo()
    : string_type(std::make_shared<TString>()) {}
bool StringValueTypeInfo::isMatch(Value *value) {
    return dynamic_cast<StringValue *>(value);
}
PType StringValueTypeInfo::getType(Value *value) {
    return std::make_shared<TString>();
}
