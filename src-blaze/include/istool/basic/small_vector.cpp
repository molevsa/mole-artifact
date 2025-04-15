//
// Created by pro on 2024/10/4.
//

#include "small_vector.h"
#include <cassert>
#include "glog/logging.h"

template <typename T>
ArrayIterator<T>::ArrayIterator(T *_ptr): ptr(_ptr) {}

template <typename T>
ArrayIterator<T>::ArrayIterator(): ptr(nullptr) {}

template <typename T>
bool ArrayIterator<T>::operator!=(const ArrayIterator<T> &other) const {
    return ptr != other.ptr;
}

template <typename T>
T &ArrayIterator<T>::operator*() const {
    return *ptr;
}

template <typename T>
T *ArrayIterator<T>::operator->() {
    return ptr;
}

template <typename T>
ArrayIterator<T> &ArrayIterator<T>::operator++() {
    ++ptr; return *this;
}

template <typename T>
ArrayIterator<T> ArrayIterator<T>::operator++(int) {
    auto copy = *this;
    ++ptr; return copy;
}

template <typename T>
bool ArrayIterator<T>::operator==(const ArrayIterator<T> &other) const {
    return ptr == other.ptr;
}

template <typename T>
FixedVector<T>::FixedVector(int _size): len(_size) {
    content = new T[len];
}

template <typename T>
FixedVector<T>::FixedVector(): len(0), content(nullptr) {
}

template <typename T>
FixedVector<T>::~FixedVector() {
    delete[] content;
}

template <typename T>
T &FixedVector<T>::operator[](int index) const {
#ifdef DEBUG
    assert(index >= 0 && index < len);
#endif
    return content[index];
}

template <typename T>
T &FixedVector<T>::at(int index) {
#ifdef DEBUG
    assert(index >= 0 && index < len);
#endif
    return content[index];
}

template <typename T>
FixedVector<T>::FixedVector(FixedVector<T> &&other): len(other.len), content(other.content) {
    other.len = 0; other.content = nullptr;
}

template <typename T>
ArrayIterator<T> FixedVector<T>::begin() const {
    return ArrayIterator<T>(content);
}

template <typename T>
ArrayIterator<T> FixedVector<T>::end() const {
    return ArrayIterator<T>(content + len);
}

template <typename T>
int FixedVector<T>::size() const {
    return len;
}

template <typename T>
FixedVector<T> &FixedVector<T>::operator=(FixedVector<T> &&other) {
#ifdef DEBUG
    if (content == other.content) {
        LOG(INFO) << "equal content " << content << " " << other.content;
    }
    assert(content != other.content);
#endif
    delete[] content;
    content = other.content; len = other.len;
    other.len = 0; other.content = nullptr;
    return *this;
}

template <typename T>
FixedVector<T> FixedVector<T>::clone() const {
    FixedVector<T> result(len);
    std::copy(content, content + len, result.content);
    return result;
}

template <typename T>
SmallVector<T>::SmallVector(int _size): len(_size), n(0) {
    content = new T[len];
}

template <typename T>
SmallVector<T>::SmallVector(): len(0), n(0), content(nullptr) {
}

template <typename T>
int SmallVector<T>::size() const {
    return n;
}

template <typename T>
SmallVector<T>::~SmallVector() {
    delete[] content;
}

template <typename T>
void SmallVector<T>::append(T &&element) {
    if (n == len) {
#ifdef DEBUG
        assert(len);
#endif
        len <<= 1;
        auto* new_content = new T[len];
        std::move(content, content + n, new_content);
        delete[] content;
        content = new_content;
    }
    content[n++] = std::move(element);
}

template <typename T>
void SmallVector<T>::copyInto(const T &element) {
    if (n == len) {
        len <<= 1;
        auto* new_content = new T[len];
        std::move(content, content + n, new_content);
        delete[] content;
        content = new_content;
    }
    content[n++] = element;
}

template <typename T>
ArrayIterator<T> SmallVector<T>::begin() const {
    return ArrayIterator<T>(content);
}

template <typename T>
ArrayIterator<T> SmallVector<T>::end() const {
    return ArrayIterator<T>(content + n);
}

template <typename T>
SmallVector<T>& SmallVector<T>::operator=(SmallVector<T> &&other) {
#ifdef DEBUG
    assert(content != other.content);
#endif
    delete[] content;
    content = other.content; len = other.len; n = other.n;
    other.content = nullptr; other.len = 0; other.n = 0;
    return *this;
}

template <typename T>
SmallVector<T>::SmallVector(SmallVector<T> &&other): content(other.content), len(other.len), n(other.n) {
    other.content = nullptr; other.len = 0; other.n = 0;
}

template <typename T>
FixedVector<T> SmallVector<T>::cloneIntoFix() const {
    FixedVector<T> res(n);
    std::copy(content, content + n, res.begin());
    return res;
}

template <typename T>
T &SmallVector<T>::operator[](int index) const {
#ifdef DEBUG
    assert(index >= 0 && index < n);
#endif
    return content[index];
}

template <typename T>
T &SmallVector<T>::at(int index) {
#ifdef DEBUG
    assert(index >= 0 && index < n);
#endif
    return content[index];
}

template <typename T>
bool FixedVectorCmp<T>::operator () (const FixedVector<T> &x, const FixedVector<T> &y) const {
    return getSign(x, y) < 0;
}

template <typename T>
int FixedVectorCmp<T>::getSign(const FixedVector<T> &x, const FixedVector<T> &y) const {
#ifdef DEBUG
    assert(x.size() == y.size());
#endif
    for (int i = 0; i < x.size(); ++i) {
        if (x[i] < y[i]) return -1;
        if (x[i] > y[i]) return 1;
    }
    return 0;
}