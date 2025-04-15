//
// Created by pro on 2024/10/4.
//

#ifndef ISTOOL_SMALL_VECTOR_H
#define ISTOOL_SMALL_VECTOR_H

#include <memory>

template <typename T>
class ArrayIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;

    ArrayIterator(T* ptr);
    ArrayIterator();
    T& operator*() const;
    T* operator->();

    ArrayIterator& operator++();
    ArrayIterator operator++(int);
    bool operator == (const ArrayIterator& other) const;
    bool operator != (const ArrayIterator& other) const;
private:
    T* ptr;
};

template<class T>
class FixedVector {
    int len;
public:
    T* content;
    using iterator = ArrayIterator<T>;
    FixedVector(int _size);
    FixedVector();
    ~FixedVector();
    FixedVector (const FixedVector&) = delete;
    FixedVector& operator = (const FixedVector&) = delete;
    FixedVector (FixedVector&& other);
    FixedVector clone() const;
    FixedVector& operator = (FixedVector&& other);
    T& operator [] (int index) const;
    T& at(int index);
    int size() const;

    ArrayIterator<T> begin() const;
    ArrayIterator<T> end() const;
};

template<class T>
class FixedVectorCmp {
public:
    bool operator () (const FixedVector<T>& x, const FixedVector<T>& y) const;
    int getSign(const FixedVector<T>& x, const FixedVector<T>& y) const;
};

template<class T>
class SmallVector {
public:
    int n;
    T* content;
    int len;
    using iterator = ArrayIterator<T>;
    SmallVector(int _size);
    SmallVector();
    ~SmallVector();
    SmallVector(const SmallVector&) = delete;
    SmallVector& operator = (const SmallVector&) = delete;
    SmallVector(SmallVector&& other);
    SmallVector& operator = (SmallVector&& other);
    FixedVector<T> cloneIntoFix() const;
    T& operator [] (int index) const;
    T& at(int index);
    int size() const;
    void append(T&& element);
    void copyInto(const T& element);

    ArrayIterator<T> begin() const;
    ArrayIterator<T> end() const;
};


#include "istool/basic/small_vector.cpp"

#endif //ISTOOL_SMALL_VECTOR_H
