#include "HeapCounter.hpp"

#include <cstdlib>
#include <malloc.h>
#include <new>

namespace {

size_t heapBytes = 0;
size_t heapAllocations = 0;

}

size_t HeapBytes() { return heapBytes; }
size_t HeapAllocations() { return heapAllocations; }

void* operator new(size_t size) {
    void* p = std::malloc(size == 0 ? 1 : size);
    if (p == nullptr) {
        throw std::bad_alloc();
    }
    heapBytes += malloc_usable_size(p) + sizeof(void*);
    ++heapAllocations;
    return p;
}

void operator delete(void* p) noexcept {
    if (p == nullptr) {
        return;
    }
    heapBytes -= malloc_usable_size(p) + sizeof(void*);
    std::free(p);
}

void operator delete(void* p, size_t) noexcept { operator delete(p); }
