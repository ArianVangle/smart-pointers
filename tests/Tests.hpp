#pragma once

#include <cassert>
#include <exception>
#include <iostream>

struct Tracked {
    inline static int alive = 0;
    int value;
    explicit Tracked(int v = 0) : value(v) { ++alive; }
    Tracked(const Tracked& other) : value(other.value) { ++alive; }
    Tracked& operator=(const Tracked&) = default;
    virtual ~Tracked() { --alive; }
};

struct Derived : Tracked {
    explicit Derived(int v = 0) : Tracked(v) {}
};

struct Unrelated {};

struct PlainA {
    inline static int destroyed = 0;
    int a = 1;
    ~PlainA() { ++destroyed; }
};

struct PlainB {
    inline static int destroyed = 0;
    int b = 2;
    ~PlainB() { ++destroyed; }
};

struct PlainC : PlainA, PlainB {
    inline static int destroyed = 0;
    int c = 3;
    ~PlainC() { ++destroyed; }
};

struct PlainMid : PlainA {
    int m = 4;
};

struct PlainLeaf : PlainMid {
    inline static int destroyed = 0;
    ~PlainLeaf() { ++destroyed; }
};

void RunTest(const char* name, void (*test)());

template <typename Func>
bool Throws(Func func) {
    try {
        func();
    } catch (const std::exception&) {
        return true;
    }
    return false;
}

void RunUnqPtrTests();
void RunShrdPtrTests();
void RunDynamicArrayTests();
void RunMemorySpanTests();
void RunMsPtrTests();

void RunAllTests();