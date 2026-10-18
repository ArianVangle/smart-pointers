#include "MemorySpan.hpp"
#include "MsPtr.hpp"
#include "Tests.hpp"

namespace {

MemorySpan<int> MakeFilled(size_t n) {
    MemorySpan<int> span(n);
    for (size_t i = 0; i < n; ++i) {
        span.Set(i, static_cast<int>(i));
    }
    return span;
}

void TestReadWrite() {
    MemorySpan<int> span(5);
    MsPtr<int> p = span.Locate(1);
    *p = 10;
    assert(*p == 10);
    assert(p.Index() == 1);
    assert(p.IsValid());
}

void TestArithmetic() {
    MemorySpan<int> span = MakeFilled(10);
    MsPtr<int> p = span.Locate(0);
    assert(*(p + 3) == 3);
    p += 5;
    assert(*p == 5);
    ++p;
    assert(*p == 6);
    p++;
    assert(*p == 7);
    --p;
    assert(*p == 6);
    p--;
    assert(*p == 5);
    p -= 1;
    assert(*p == 4);
    assert(p[2] == 6);
    assert(p[-4] == 0);
    assert((p + 4) - p == 4);
    assert(p - span.Locate(0) == 4);
    assert(*(p - 1) == 3);
}

void TestComparison() {
    MemorySpan<int> span = MakeFilled(10);
    MsPtr<int> p = span.Locate(3);
    assert(p == span.Locate(3));
    assert(p != span.Locate(4));
    assert(p < p + 1);
    assert(p + 1 > p);
    assert(p <= span.Locate(3));
    assert(p >= span.Locate(3));
}

void TestIteration() {
    MemorySpan<int> span = MakeFilled(10);
    MsPtr<int> end = span.Locate(0) + 10;
    int sum = 0;
    for (MsPtr<int> it = span.Locate(0); it != end; ++it) {
        sum += *it;
    }
    assert(sum == 45);
    assert(!end.IsValid());
}

void TestBounds() {
    MemorySpan<int> span = MakeFilled(10);
    assert(Throws([&] { span.Locate(0) - 1; }));
    assert(Throws([&] { span.Locate(0) + 11; }));
    assert(Throws([&] { span.Locate(9) + 2; }));
    assert(Throws([&] {
        MsPtr<int> p = span.Locate(9);
        ++p;
        ++p;
    }));
    assert(Throws([&] {
        MsPtr<int> p = span.Locate(0);
        --p;
    }));
    assert(Throws([&] { *(span.Locate(0) + 10); }));
    assert(Throws([&] { span.Locate(0)[10]; }));
    assert(Throws([&] { span.Locate(5)[-6]; }));
}

void TestFailedMoveKeepsPosition() {
    MemorySpan<int> span = MakeFilled(5);
    MsPtr<int> p = span.Locate(4);
    assert(Throws([&] { p += 10; }));
    assert(p.Index() == 4);
    assert(*p == 4);
}

void TestNullPointer() {
    MsPtr<int> empty;
    assert(!empty.IsValid());
    assert(Throws([&] { *empty; }));
    assert(Throws([&] { empty + 1; }));
}

void TestForeignPointers() {
    MemorySpan<int> a = MakeFilled(3);
    MemorySpan<int> b = MakeFilled(3);
    assert(Throws([&] { a.Locate(0) - b.Locate(0); }));
    assert(Throws([&] { return a.Locate(0) < b.Locate(1); }));
    assert(Throws([&] { return a.Locate(2) >= b.Locate(0); }));
    assert(a.Locate(1) != b.Locate(1));
}

void TestArrowOperator() {
    MemorySpan<Tracked> span(3);
    span.Locate(1)->value = 8;
    assert(span.Locate(1)->value == 8);
    assert(span.Get(1)->value == 8);
}

}

void RunMsPtrTests() {
    std::cout << "MsPtr\n";
    RunTest("чтение и запись", TestReadWrite);
    RunTest("арифметика", TestArithmetic);
    RunTest("сравнения", TestComparison);
    RunTest("обход от начала до end", TestIteration);
    RunTest("выход за границы", TestBounds);
    RunTest("неудачный сдвиг не меняет позицию", TestFailedMoveKeepsPosition);
    RunTest("пустой указатель", TestNullPointer);
    RunTest("указатели из разных массивов", TestForeignPointers);
    RunTest("operator->", TestArrowOperator);
}
