//
// Created by pro on 2021/12/19.
//

#ifndef ISTOOL_CLIA_VALUE_H
#define ISTOOL_CLIA_VALUE_H

#include "clia_type.h"
#include "istool/basic/type_system.h"
#include "istool/basic/value.h"

class IntValue : public Value, public ComparableValue {
public:
    std::optional<int> w;
    IntValue();
    IntValue(int _w);
    virtual std::string toString() const;
    virtual bool equal(Value *value) const;
    virtual bool leq(Value *value) const;
    virtual bool isSubValue(Value *value) const;
};

class IntValueTypeInfo : public ValueTypeInfo {
    PType int_type;

public:
    IntValueTypeInfo();
    virtual bool isMatch(Value *value);
    virtual PType getType(Value *value);
    virtual ~IntValueTypeInfo() = default;
};

namespace theory {
    namespace clia {
        std::optional<int> getIntValue(const Data &data);
    }
} // namespace theory

#endif // ISTOOL_CLIA_VALUE_H
