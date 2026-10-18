#include <type_traits>

#include "MemorySpan.hpp"
#include "Tests.hpp"

static_assert(!std::is_copy_constructible_v<MemorySpan<int>>);
static_assert(!std::is_copy_assignable_v<MemorySpan<int>>);

namespace {

void TestLengthAndDefaults() {
    MemorySpan<int> span(5);
    assert(span.GetLength() == 5);
    assert(*span.Get(0) == 0);
    assert(*span.Get(4) == 0);
}

void TestEmptySpan() {
    MemorySpan<int> span(0);
    assert(span.GetLength() == 0);
    assert(Throws([&] { span.Get(0); }));
    assert(Throws([&] { span.Locate(0); }));
}

void TestGetReturnsIndependentCopy() {
    MemorySpan<int> span(5);
    span.Set(2, 42);
    UnqPtr<int> value = span.Get(2);
    assert(*value == 42);
    *value = 7;
    assert(*span.Get(2) == 42);
}

void TestCopyReturnsShrdPtr() {
    MemorySpan<int> span(5);
    span.Set(1, 15);
    ShrdPtr<int> a = span.Copy(1);
    assert(*a == 15);
    assert(a.UseCount() == 1);
    ShrdPtr<int> b = a;
    assert(a.UseCount() == 2);
    *b = 99;
    assert(*a == 99);
    assert(*span.Get(1) == 15);
}

void TestLocateWritesIntoSpan() {
    MemorySpan<int> span(5);
    MsPtr<int> p = span.Locate(1);
    *p = 10;
    assert(*span.Get(1) == 10);
}

void TestBounds() {
    MemorySpan<int> span(10);
    assert(Throws([&] { span.Locate(10); }));
    assert(Throws([&] { span.Get(10); }));
    assert(Throws([&] { span.Copy(10); }));
    assert(Throws([&] { span.Set(10, 1); }));
}

void TestNoLeaks() {
    {
        MemorySpan<Tracked> span(5);
        assert(Tracked::alive == 5);
        UnqPtr<Tracked> u = span.Get(0);
        assert(Tracked::alive == 6);
        ShrdPtr<Tracked> s = span.Copy(1);
        ShrdPtr<Tracked> s2 = s;
        assert(Tracked::alive == 7);
    }
    assert(Tracked::alive == 0);
    for (int i = 0; i < 1000; ++i) {
        MemorySpan<Tracked> span(10);
        ShrdPtr<Tracked> s = span.Copy(static_cast<size_t>(i % 10));
    }
    assert(Tracked::alive == 0);
}

}

void RunMemorySpanTests() {
    std::cout << "MemorySpan\n";
    RunTest("длина и значения по умолчанию", TestLengthAndDefaults);
    RunTest("пустой массив", TestEmptySpan);
    RunTest("Get возвращает независимую копию", TestGetReturnsIndependentCopy);
    RunTest("Copy возвращает ShrdPtr", TestCopyReturnsShrdPtr);
    RunTest("Locate пишет в сам массив", TestLocateWritesIntoSpan);
    RunTest("выход за границы", TestBounds);
    RunTest("без утечек", TestNoLeaks);
}
