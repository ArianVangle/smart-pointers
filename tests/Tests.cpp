#include "Tests.hpp"

namespace {

int testsPassed = 0;

void ResetCounters() {
    Tracked::alive = 0;
    PlainA::destroyed = 0;
    PlainB::destroyed = 0;
    PlainC::destroyed = 0;
    PlainLeaf::destroyed = 0;
}

}

void RunTest(const char* name, void (*test)()) {
    ResetCounters();
    test();
    assert(Tracked::alive == 0);
    ++testsPassed;
    std::cout << "  ok  " << name << "\n";
}

void RunAllTests() {
    testsPassed = 0;
    RunUnqPtrTests();
    RunShrdPtrTests();
    RunDynamicArrayTests();
    RunMemorySpanTests();
    RunMsPtrTests();
    std::cout << "\nВсе тесты пройдены: " << testsPassed << std::endl;
}