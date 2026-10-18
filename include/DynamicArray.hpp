#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "UnqPtr.hpp"

template <typename T>
class DynamicArray {
public:
    DynamicArray() : data_(), size_(0), capacity_(0) {}

    void Append(T value) {
        if (size_ == capacity_) {
            Grow();
        }
        data_[size_] = std::move(value);
        ++size_;
    }

    T& Get(size_t index) {
        CheckIndex(index);
        return data_[index];
    }

    const T& Get(size_t index) const {
        CheckIndex(index);
        return data_[index];
    }

    T& operator[](size_t index) { return Get(index); }
    const T& operator[](size_t index) const { return Get(index); }

    size_t GetLength() const { return size_; }

private:
    void CheckIndex(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index out of range");
        }
    }

    void Grow() {
        size_t newCapacity = capacity_ == 0 ? 4 : capacity_ * 2;
        UnqPtr<T[]> fresh(new T[newCapacity]);
        for (size_t i = 0; i < size_; ++i) {
            fresh[i] = std::move(data_[i]);
        }
        data_ = std::move(fresh);
        capacity_ = newCapacity;
    }

    UnqPtr<T[]> data_;
    size_t size_;
    size_t capacity_;
};
