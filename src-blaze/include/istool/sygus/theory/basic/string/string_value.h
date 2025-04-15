//
// Created by pro on 2021/12/28.
//

#ifndef ISTOOL_STRING_VALUE_H
#define ISTOOL_STRING_VALUE_H

#include <map>
#include <optional>

#include "istool/basic/data.h"
#include "istool/basic/type_system.h"
#include "istool/basic/value.h"

enum class StringValueType { CONDITION, TRUE, FALSE };

class StringValue : public Value {
public:
    StringValueType sv_type;
    std::optional<int> length;
    std::map<int, char> at;
    StringValue(std::string);
    StringValue(StringValue *);
    StringValue(StringValueType);
    bool operator<(const StringValue &) const;
    virtual std::string toString() const;
    virtual bool equal(Value *value) const;
    bool isFullString() const;
    virtual bool isSubValue(Value *value) const;
    std::optional<std::string> getFullString() const;
};

class StringValueTypeInfo : public ValueTypeInfo {
    PType string_type;

public:
    StringValueTypeInfo();
    virtual bool isMatch(Value *value);
    virtual PType getType(Value *value);
    virtual ~StringValueTypeInfo() = default;
};

namespace theory {
    namespace string {
        StringValue getStringValue(const Data &d);
    }
} // namespace theory

#endif // ISTOOL_STRING_VALUE_H
