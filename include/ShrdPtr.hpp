#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "UnqPtr.hpp"

template <typename T>
class ShrdPtr {
public:
    ShrdPtr() : master_(nullptr), referenceCount_(nullptr) {}

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    ShrdPtr(UnqPtr<U>&& master) : master_(nullptr), referenceCount_(nullptr) {
        if (master) {
            master_ = new UnqPtr<T>(std::move(master));
            referenceCount_ = new size_t(1);
        }
    }

    ShrdPtr(const ShrdPtr& other)
        : master_(other.master_), referenceCount_(other.referenceCount_) {
        if (referenceCount_ != nullptr) {
            ++(*referenceCount_);
        }
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    ShrdPtr(const ShrdPtr<U>& other) : master_(nullptr), referenceCount_(nullptr) {
        if (other) {
            ShrdPtr<U>* keeper = new ShrdPtr<U>(other);
            master_ = new UnqPtr<T>(other.Get(), keeper, &DeleteObject<ShrdPtr<U>>);
            referenceCount_ = new size_t(1);
        }
    }

    ShrdPtr(ShrdPtr&& other) noexcept
        : master_(other.master_), referenceCount_(other.referenceCount_) {
        other.master_ = nullptr;
        other.referenceCount_ = nullptr;
    }

    ShrdPtr& operator=(const ShrdPtr& other) {
        ShrdPtr copy(other);
        Swap(copy);
        return *this;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    ShrdPtr& operator=(const ShrdPtr<U>& other) {
        ShrdPtr copy(other);
        Swap(copy);
        return *this;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this != &other) {
            Release();
            master_ = other.master_;
            referenceCount_ = other.referenceCount_;
            other.master_ = nullptr;
            other.referenceCount_ = nullptr;
        }
        return *this;
    }

    ~ShrdPtr() { Release(); }

    T& operator*() const { return *Get(); }
    T* operator->() const { return Get(); }
    T* Get() const { return master_ == nullptr ? nullptr : master_->Get(); }

    size_t UseCount() const {
        return referenceCount_ == nullptr ? 0 : *referenceCount_;
    }

    explicit operator bool() const { return Get() != nullptr; }

    void Swap(ShrdPtr& other) noexcept {
        std::swap(master_, other.master_);
        std::swap(referenceCount_, other.referenceCount_);
    }

private:
    void Release() {
        if (referenceCount_ != nullptr) {
            --(*referenceCount_);
            if (*referenceCount_ == 0) {
                delete master_;
                delete referenceCount_;
            }
        }
        master_ = nullptr;
        referenceCount_ = nullptr;
    }

    UnqPtr<T>* master_;
    size_t* referenceCount_;
};
