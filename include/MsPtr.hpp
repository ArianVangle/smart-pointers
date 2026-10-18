#pragma once

#include <compare>
#include <cstddef>
#include <stdexcept>

template <typename T>
class MemorySpan;

template <typename T>
class MsPtr {
public:
    MsPtr() : span_(nullptr), index_(0) {}

    T& operator*() const { return Element(); }
    T* operator->() const { return &Element(); }
    T& operator[](ptrdiff_t offset) const { return *(*this + offset); }

    MsPtr& operator+=(ptrdiff_t offset) {
        Move(offset);
        return *this;
    }

    MsPtr& operator-=(ptrdiff_t offset) {
        Move(-offset);
        return *this;
    }

    MsPtr operator+(ptrdiff_t offset) const {
        MsPtr result(*this);
        result += offset;
        return result;
    }

    MsPtr operator-(ptrdiff_t offset) const {
        MsPtr result(*this);
        result -= offset;
        return result;
    }

    ptrdiff_t operator-(const MsPtr& other) const {
        if (span_ != other.span_) {
            throw std::invalid_argument("указатели относятся к разным MemorySpan");
        }
        return static_cast<ptrdiff_t>(index_) -
               static_cast<ptrdiff_t>(other.index_);
    }

    MsPtr& operator++() { return *this += 1; }
    MsPtr& operator--() { return *this -= 1; }

    MsPtr operator++(int) {
        MsPtr old(*this);
        ++*this;
        return old;
    }

    MsPtr operator--(int) {
        MsPtr old(*this);
        --*this;
        return old;
    }

    bool operator==(const MsPtr& other) const {
        return span_ == other.span_ && index_ == other.index_;
    }

    std::strong_ordering operator<=>(const MsPtr& other) const {
        if (span_ != other.span_) {
            throw std::invalid_argument("указатели относятся к разным MemorySpan");
        }
        return index_ <=> other.index_;
    }

    size_t Index() const { return index_; }

    bool IsValid() const {
        return span_ != nullptr && index_ < span_->GetLength();
    }

private:
    friend class MemorySpan<T>;

    MsPtr(MemorySpan<T>* span, size_t index) : span_(span), index_(index) {}

    T& Element() const {
        if (!IsValid()) {
            throw std::out_of_range("MsPtr не указывает на элемент");
        }
        return span_->ElementAt(index_);
    }

    void Move(ptrdiff_t offset) {
        if (span_ == nullptr) {
            throw std::out_of_range("MsPtr не привязан к MemorySpan");
        }
        ptrdiff_t target = static_cast<ptrdiff_t>(index_) + offset;
        if (target < 0 || target > static_cast<ptrdiff_t>(span_->GetLength())) {
            throw std::out_of_range("MsPtr вышел за границы MemorySpan");
        }
        index_ = static_cast<size_t>(target);
    }

    MemorySpan<T>* span_;
    size_t index_;
};
