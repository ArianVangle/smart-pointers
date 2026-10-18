#pragma once

#include <cstddef>
#include <stdexcept>

#include "MsPtr.hpp"
#include "ShrdPtr.hpp"
#include "UnqPtr.hpp"

template <typename T>
class MemorySpan {
public:
    explicit MemorySpan(size_t size) : data_(new T[size]()), size_(size) {}

    size_t GetLength() const { return size_; }

    void Set(size_t index, const T& value) {
        CheckIndex(index);
        data_[index] = value;
    }

    UnqPtr<T> Get(size_t index) const {
        CheckIndex(index);
        return UnqPtr<T>(new T(data_[index]));
    }

    ShrdPtr<T> Copy(size_t index) const { return ShrdPtr<T>(Get(index)); }

    MsPtr<T> Locate(size_t index) {
        CheckIndex(index);
        return MsPtr<T>(this, index);
    }

private:
    friend class MsPtr<T>;

    void CheckIndex(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index out of range");
        }
    }

    T& ElementAt(size_t index) { return data_[index]; }

    UnqPtr<T[]> data_;
    size_t size_;
};
