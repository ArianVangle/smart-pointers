#include <type_traits>
#include <utility>

#include "Tests.hpp"
#include "UnqPtr.hpp"

static_assert(!std::is_copy_constructible_v<UnqPtr<int>>);
static_assert(!std::is_copy_assignable_v<UnqPtr<int>>);
static_assert(!std::is_copy_constructible_v<UnqPtr<int[]>>);
static_assert(!std::is_copy_assignable_v<UnqPtr<int[]>>);
static_assert(std::is_move_constructible_v<UnqPtr<int>>);
static_assert(std::is_move_assignable_v<UnqPtr<int>>);
static_assert(std::is_convertible_v<UnqPtr<Derived>, UnqPtr<Tracked>>);
static_assert(!std::is_convertible_v<UnqPtr<Tracked>, UnqPtr<Derived>>);
static_assert(!std::is_convertible_v<UnqPtr<Unrelated>, UnqPtr<Tracked>>);
static_assert(!std::is_assignable_v<UnqPtr<Derived>&, UnqPtr<Tracked>>);
static_assert(!std::is_convertible_v<Tracked*, UnqPtr<Tracked>>);
static_assert(!std::is_constructible_v<UnqPtr<Tracked>, Unrelated*>);
static_assert(!std::is_constructible_v<UnqPtr<Derived>, Tracked*>);
static_assert(std::is_constructible_v<UnqPtr<Tracked[]>, Tracked*>);
static_assert(!std::is_constructible_v<UnqPtr<Tracked[]>, Derived*>);

namespace {

void TestBasicAccess() {
    UnqPtr<Tracked> p(new Tracked(5));
    assert(Tracked::alive == 1);
    assert(p->value == 5);
    assert((*p).value == 5);
    assert(p.Get() != nullptr);
    assert(p);
}

void TestEmpty() {
    UnqPtr<Tracked> p;
    assert(!p);
    assert(p.Get() == nullptr);
    assert(p.Release() == nullptr);
    p.Reset();
    assert(!p);
}

void TestMove() {
    UnqPtr<Tracked> a(new Tracked(1));
    UnqPtr<Tracked> b(std::move(a));
    assert(!a);
    assert(b->value == 1);
    UnqPtr<Tracked> c;
    c = std::move(b);
    assert(!b);
    assert(c->value == 1);
    assert(Tracked::alive == 1);
}

void TestMoveAssignReleasesOld() {
    UnqPtr<Tracked> a(new Tracked(1));
    UnqPtr<Tracked> b(new Tracked(2));
    a = std::move(b);
    assert(Tracked::alive == 1);
    assert(a->value == 2);
}

void TestSelfMoveAssign() {
    UnqPtr<Tracked> a(new Tracked(3));
    UnqPtr<Tracked>& alias = a;
    a = std::move(alias);
    assert(a);
    assert(a->value == 3);
    assert(Tracked::alive == 1);
}

void TestRelease() {
    UnqPtr<Tracked> p(new Tracked(3));
    Tracked* raw = p.Release();
    assert(!p);
    assert(Tracked::alive == 1);
    delete raw;
    assert(Tracked::alive == 0);
}

void TestReset() {
    UnqPtr<Tracked> p(new Tracked(1));
    p.Reset(new Tracked(2));
    assert(Tracked::alive == 1);
    assert(p->value == 2);
    p.Reset();
    assert(!p);
    assert(Tracked::alive == 0);
}

void TestArray() {
    UnqPtr<Tracked[]> arr(new Tracked[4]);
    assert(Tracked::alive == 4);
    for (size_t i = 0; i < 4; ++i) {
        arr[i].value = static_cast<int>(i * 10);
    }
    assert(arr[3].value == 30);
}

void TestArrayMoveAndReset() {
    UnqPtr<Tracked[]> a(new Tracked[3]);
    UnqPtr<Tracked[]> b(std::move(a));
    assert(!a);
    assert(Tracked::alive == 3);
    b.Reset(new Tracked[2]);
    assert(Tracked::alive == 2);
    b.Reset();
    assert(Tracked::alive == 0);
}

void TestSubtyping() {
    UnqPtr<Derived> d(new Derived(7));
    UnqPtr<Tracked> t(std::move(d));
    assert(!d);
    assert(t->value == 7);
    UnqPtr<Tracked> assigned;
    assigned = UnqPtr<Derived>(new Derived(8));
    assert(assigned->value == 8);
}

void TestMultipleInheritanceNoVirtual() {
    {
        UnqPtr<PlainC> c(new PlainC);
        UnqPtr<PlainB> b(std::move(c));
        assert(!c);
        assert(b->b == 2);
        assert(PlainC::destroyed == 0);
    }
    assert(PlainC::destroyed == 1);
    assert(PlainA::destroyed == 1);
    assert(PlainB::destroyed == 1);
}

void TestChainNoVirtual() {
    {
        UnqPtr<PlainLeaf> leaf(new PlainLeaf);
        UnqPtr<PlainMid> mid(std::move(leaf));
        UnqPtr<PlainA> base(std::move(mid));
        assert(base->a == 1);
    }
    assert(PlainLeaf::destroyed == 1);
}

void TestAssignNoVirtual() {
    UnqPtr<PlainB> b;
    b = UnqPtr<PlainC>(new PlainC);
    assert(PlainC::destroyed == 0);
    b.Reset();
    assert(PlainC::destroyed == 1);
}

void TestBaseFromNewNoVirtual() {
    {
        UnqPtr<PlainB> b(new PlainC);
        assert(b->b == 2);
    }
    assert(PlainC::destroyed == 1);
    assert(PlainA::destroyed == 1);
    assert(PlainB::destroyed == 1);
}

void TestResetWithDerivedNoVirtual() {
    UnqPtr<PlainB> b;
    b.Reset(new PlainC);
    b.Reset(new PlainC);
    assert(PlainC::destroyed == 1);
    b.Reset();
    assert(PlainC::destroyed == 2);
}

void TestNoLeaks() {
    for (int i = 0; i < 1000; ++i) {
        UnqPtr<Tracked> single(new Tracked(i));
        UnqPtr<Tracked[]> many(new Tracked[3]);
        UnqPtr<Tracked> moved(std::move(single));
    }
    assert(Tracked::alive == 0);
}

}

void RunUnqPtrTests() {
    std::cout << "UnqPtr\n";
    RunTest("доступ через *, -> и Get", TestBasicAccess);
    RunTest("пустой указатель", TestEmpty);
    RunTest("перемещение", TestMove);
    RunTest("перемещающее присваивание удаляет старый объект", TestMoveAssignReleasesOld);
    RunTest("самоперемещение", TestSelfMoveAssign);
    RunTest("Release отдаёт владение", TestRelease);
    RunTest("Reset заменяет и удаляет объект", TestReset);
    RunTest("массив и operator[]", TestArray);
    RunTest("массив: перемещение и Reset", TestArrayMoveAndReset);
    RunTest("подтипизация Derived -> Base", TestSubtyping);
    RunTest("множественное наследование без virtual", TestMultipleInheritanceNoVirtual);
    RunTest("цепочка Leaf -> Mid -> Base без virtual", TestChainNoVirtual);
    RunTest("присваивание наследника без virtual", TestAssignNoVirtual);
    RunTest("указатель на вторую базу сразу из new без virtual", TestBaseFromNewNoVirtual);
    RunTest("Reset наследником без virtual", TestResetWithDerivedNoVirtual);
    RunTest("1000 циклов без утечек", TestNoLeaks);
}