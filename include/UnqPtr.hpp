#pragma once

#include <cstddef>
#include <type_traits>

template <typename T>
void DeleteObject(void* object) {
    delete static_cast<T*>(object);
}

template <typename T>
class ShrdPtr;

template <typename T>
class UnqPtr {
public:
    UnqPtr() : ptr_(nullptr), owned_(nullptr), deleter_(nullptr) {}

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    explicit UnqPtr(U* ptr)
        : ptr_(ptr),
          owned_(ptr),
          deleter_(ptr != nullptr ? &DeleteObject<U> : nullptr) {}

    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    UnqPtr(UnqPtr&& other) noexcept
        : ptr_(other.ptr_), owned_(other.owned_), deleter_(other.deleter_) {
        other.Clear();
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    UnqPtr(UnqPtr<U>&& other) noexcept
        : ptr_(other.ptr_), owned_(other.owned_), deleter_(other.deleter_) {
        other.Clear();
    }

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            Destroy();
            ptr_ = other.ptr_;
            owned_ = other.owned_;
            deleter_ = other.deleter_;
            other.Clear();
        }
        return *this;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    UnqPtr& operator=(UnqPtr<U>&& other) noexcept {
        Destroy();
        ptr_ = other.ptr_;
        owned_ = other.owned_;
        deleter_ = other.deleter_;
        other.Clear();
        return *this;
    }

    ~UnqPtr() { Destroy(); }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    T* Get() const { return ptr_; }

    T* Release() {
        T* result = ptr_;
        Clear();
        return result;
    }

    void Reset() { Destroy(); }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    void Reset(U* ptr) {
        if (ptr_ != ptr) {
            Destroy();
            ptr_ = ptr;
            owned_ = ptr;
            deleter_ = ptr != nullptr ? &DeleteObject<U> : nullptr;
        }
    }

    explicit operator bool() const { return ptr_ != nullptr; }

private:
    template <typename U>
    friend class UnqPtr;

    template <typename U>
    friend class ShrdPtr;

    UnqPtr(T* ptr, void* owned, void (*deleter)(void*))
        : ptr_(ptr), owned_(owned), deleter_(deleter) {}

    void Clear() {
        ptr_ = nullptr;
        owned_ = nullptr;
        deleter_ = nullptr;
    }

    void Destroy() {
        if (deleter_ != nullptr) {
            deleter_(owned_);
        }
        Clear();
    }

    T* ptr_;
    void* owned_;
    void (*deleter_)(void*);
};

template <typename T>
class UnqPtr<T[]> {
public:
    UnqPtr() : ptr_(nullptr) {}

    template <typename U>
        requires std::is_same_v<U, T>
    explicit UnqPtr(U* ptr) : ptr_(ptr) {}

    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    UnqPtr(UnqPtr&& other) noexcept : ptr_(other.Release()) {}

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            Reset(other.Release());
        }
        return *this;
    }

    ~UnqPtr() { delete[] ptr_; }

    T& operator[](size_t index) const { return ptr_[index]; }
    T* Get() const { return ptr_; }

    T* Release() {
        T* result = ptr_;
        ptr_ = nullptr;
        return result;
    }

    void Reset() {
        delete[] ptr_;
        ptr_ = nullptr;
    }

    template <typename U>
        requires std::is_same_v<U, T>
    void Reset(U* ptr) {
        if (ptr_ != ptr) {
            delete[] ptr_;
            ptr_ = ptr;
        }
    }

    explicit operator bool() const { return ptr_ != nullptr; }

private:
    T* ptr_;
};