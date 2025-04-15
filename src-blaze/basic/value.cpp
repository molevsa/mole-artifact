//
// Created by pro on 2021/11/30.
//

#include "istool/basic/value.h"

#include "glog/logging.h"

Value::Value() {}
NullValue::NullValue() {}
bool NullValue::equal(Value *value) const {
    auto *nv = dynamic_cast<NullValue *>(value);
    return nv;
}
bool NullValue::isSubValue(Value *value) const { return this->equal(value); }
std::string NullValue::toString() const { return "null"; }

BoolValue::BoolValue() : w() {}
BoolValue::BoolValue(bool _w) : w(_w) {}
bool BoolValue::equal(Value *value) const {
    auto *bv = dynamic_cast<BoolValue *>(value);
    if (!bv) {
        LOG(FATAL) << "Expected BoolValue, but get " << value->toString();
    }
    return bv->w == w;
}
bool BoolValue::isSubValue(Value *value) const {
    auto v = dynamic_cast<BoolValue *>(value);
    if (!v)
        return false;
    if (!w.has_value())
        return true;
    return w == v->w;
}
std::string BoolValue::toString() const {
    if (w.has_value())
        return w.value() ? "true" : "false";
    return "[Bool_NAC]";
}