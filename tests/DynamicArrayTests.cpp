#include "DynamicArray.hpp"
#include "ShrdPtr.hpp"
#include "Tests.hpp"
#include "UnqPtr.hpp"

namespace {

void TestEmptyArray() {
    DynamicArray<int> arr;
    assert(arr.GetLength() == 0);
    assert(Throws([&] { arr.Get(0); }));
}

void TestGrowthKeepsValues() {
    DynamicArray<int> arr;
    for (int i = 0; i < 1000; ++i) {
        arr.Append(i);
    }
    assert(arr.GetLength() == 1000);
    for (int i = 0; i < 1000; ++i) {
        assert(arr[i] == i);
    }
}

void TestOutOfRange() {
    DynamicArray<int> arr;
    arr.Append(1);
    assert(arr.Get(0) == 1);
    assert(Throws([&] { arr.Get(1); }));
    assert(Throws([&] { arr[100]; }));
}

void TestShrdPtrElements() {
    DynamicArray<ShrdPtr<Tracked>> arr;
    for (int i = 0; i < 100; ++i) {
        arr.Append(ShrdPtr<Tracked>(UnqPtr<Tracked>(new Tracked(i))));
    }
    assert(arr.GetLength() == 100);
    assert(arr[57]->value == 57);
    ShrdPtr<Tracked> shared = arr[10];
    assert(shared.UseCount() == 2);
    assert(Tracked::alive == 100);
}

void TestUnqPtrElements() {
    DynamicArray<UnqPtr<Tracked>> arr;
    for (int i = 0; i < 50; ++i) {
        arr.Append(UnqPtr<Tracked>(new Tracked(i)));
    }
    assert(arr[49]->value == 49);
    assert(Tracked::alive == 50);
}

void TestPolymorphicElements() {
    DynamicArray<ShrdPtr<Tracked>> arr;
    arr.Append(ShrdPtr<Derived>(UnqPtr<Derived>(new Derived(1))));
    arr.Append(ShrdPtr<Tracked>(UnqPtr<Tracked>(new Tracked(2))));
    assert(arr[0]->value == 1);
    assert(arr[1]->value == 2);
}

void TestMoveAssign() {
    DynamicArray<UnqPtr<Tracked>> arr;
    arr.Append(UnqPtr<Tracked>(new Tracked(1)));
    arr.Append(UnqPtr<Tracked>(new Tracked(2)));
    arr = DynamicArray<UnqPtr<Tracked>>();
    assert(arr.GetLength() == 0);
    assert(Tracked::alive == 0);
}

}

void RunDynamicArrayTests() {
    std::cout << "DynamicArray\n";
    RunTest("пустой массив", TestEmptyArray);
    RunTest("рост сохраняет значения", TestGrowthKeepsValues);
    RunTest("выход за границы", TestOutOfRange);
    RunTest("элементы ShrdPtr", TestShrdPtrElements);
    RunTest("элементы UnqPtr", TestUnqPtrElements);
    RunTest("полиморфные элементы", TestPolymorphicElements);
    RunTest("перемещающее присваивание освобождает элементы", TestMoveAssign);
}
