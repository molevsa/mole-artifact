//
// Created by pro on 2021/11/30.
//

#ifndef ISTOOL_VALUE_H
#define ISTOOL_VALUE_H

#include <memory>
#include <optional>
#include <string>

#include "type.h"

class Value {
public:
    Value();
    virtual ~Value() = default;
    virtual std::string toString() const = 0;
    virtual bool equal(Value *value) const = 0;
    virtual bool isSubValue(Value *value) const = 0;
};

typedef std::shared_ptr<Value> PValue;

class ComparableValue {
public:
    virtual bool leq(Value *value) const = 0;
};

class NullValue : public Value {
public:
    NullValue();
    virtual std::string toString() const;
    virtual bool equal(Value *value) const;
    virtual bool isSubValue(Value *value) const;
};

class BoolValue : public Value {
public:
    std::optional<bool> w;
    BoolValue();
    BoolValue(bool _w);
    virtual std::string toString() const;
    virtual bool equal(Value *value) const;
    virtual bool isSubValue(Value *value) const;
};

#endif // ISTOOL_VALUE_H
