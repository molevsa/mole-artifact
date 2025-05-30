//
// Created by pro on 2022/2/12.
//

#include "istool/sygus/theory/basic/bv/bv_value.h"

#include "glog/logging.h"
#include "istool/sygus/theory/basic/bv/bv_type.h"

BitVectorValue::BitVectorValue(const Bitset &_w) : w(_w) {}
std::string BitVectorValue::toString() const { return w.toXString(); }
bool BitVectorValue::equal(Value *value) const {
    auto *bv = dynamic_cast<BitVectorValue *>(value);
    if (!bv) {
        LOG(FATAL) << "Expect BitVectorValue, but get " << value->toString();
    }
    return w == bv->w;
}

bool BitVectorValue::isSubValue(Value *value) const {
    return this->equal(value);
}
bool BitVectorTypeInfo::isMatch(Value *value) {
    return dynamic_cast<BitVectorValue *>(value);
}
PType BitVectorTypeInfo::getType(Value *value) {
    auto *bv = dynamic_cast<BitVectorValue *>(value);
    auto &w = bv->w;
    int size = w.size();
    if (type_map.count(size))
        return type_map[size];
    return type_map[size] = std::make_shared<TBitVector>(size);
}

Bitset theory::bv::getBitVectorValue(const Data &data) {
    auto *bv = dynamic_cast<BitVectorValue *>(data.get());
    if (!bv) {
        LOG(FATAL) << "Expect BitVectorValue, but get " << data.toString();
    }
    return bv->w;
}