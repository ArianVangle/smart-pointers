#include <type_traits>
#include <utility>

#include "ShrdPtr.hpp"
#include "Tests.hpp"
#include "UnqPtr.hpp"

static_assert(std::is_convertible_v<ShrdPtr<Derived>, ShrdPtr<Tracked>>);
static_assert(!std::is_convertible_v<ShrdPtr<Tracked>, ShrdPtr<Derived>>);
static_assert(!std::is_convertible_v<ShrdPtr<Unrelated>, ShrdPtr<Tracked>>);
static_assert(std::is_assignable_v<ShrdPtr<Tracked>&, ShrdPtr<Derived>>);
static_assert(!std::is_assignable_v<ShrdPtr<Derived>&, ShrdPtr<Tracked>>);
static_assert(!std::is_assignable_v<ShrdPtr<Tracked>&, ShrdPtr<Unrelated>>);
static_assert(!std::is_constructible_v<ShrdPtr<int>, int*>);
static_assert(std::is_constructible_v<ShrdPtr<Tracked>, UnqPtr<Derived>&&>);
static_assert(!std::is_constructible_v<ShrdPtr<Tracked>, UnqPtr<Tracked>&>);
static_assert(sizeof(ShrdPtr<Tracked>) == 2 * sizeof(void*));

namespace {

void TestCount() {
    UnqPtr<Tracked> master(new Tracked(9));
    ShrdPtr<Tracked> a(std::move(master));
    assert(!master);
    assert(a.UseCount() == 1);
    {
        ShrdPtr<Tracked> b = a;
        ShrdPtr<Tracked> c(b);
        assert(a.UseCount() == 3);
        assert(c->value == 9);
    }
    assert(a.UseCount() == 1);
    assert(Tracked::alive == 1);
    a = ShrdPtr<Tracked>();
    assert(Tracked::alive == 0);
}

void TestAssign() {
    ShrdPtr<Tracked> a{UnqPtr<Tracked>(new Tracked(1))};
    ShrdPtr<Tracked> b{UnqPtr<Tracked>(new Tracked(2))};
    assert(Tracked::alive == 2);
    a = b;
    assert(Tracked::alive == 1);
    assert(a.UseCount() == 2);
    assert(a->value == 2);
}

void TestSelfAssign() {
    ShrdPtr<Tracked> a{UnqPtr<Tracked>(new Tracked(1))};
    ShrdPtr<Tracked>& alias = a;
    a = alias;
    assert(a.UseCount() == 1);
    a = std::move(alias);
    assert(a.UseCount() == 1);
    assert(a->value == 1);
}

void TestMove() {
    ShrdPtr<Tracked> a{UnqPtr<Tracked>(new Tracked(4))};
    ShrdPtr<Tracked> b(std::move(a));
    assert(!a);
    assert(a.UseCount() == 0);
    assert(b.UseCount() == 1);
    ShrdPtr<Tracked> c;
    c = std::move(b);
    assert(!b);
    assert(c.UseCount() == 1);
}

void TestEmpty() {
    ShrdPtr<Tracked> empty;
    ShrdPtr<Tracked> copy = empty;
    assert(!empty);
    assert(!copy);
    assert(copy.Get() == nullptr);
    assert(copy.UseCount() == 0);
    ShrdPtr<Tracked> fromNull{UnqPtr<Tracked>()};
    assert(!fromNull);
    assert(fromNull.UseCount() == 0);
}

void TestSubtyping() {
    ShrdPtr<Derived> d{UnqPtr<Derived>(new Derived(11))};
    ShrdPtr<Tracked> t = d;
    assert(d.UseCount() == 2);
    assert(t->value == 11);
    ShrdPtr<Tracked> fromMaster{UnqPtr<Derived>(new Derived(12))};
    assert(fromMaster->value == 12);
}

void TestBaseOutlivesDerived() {
    ShrdPtr<Tracked> base;
    {
        ShrdPtr<Derived> derived{UnqPtr<Derived>(new Derived(21))};
        base = derived;
    }
    assert(Tracked::alive == 1);
    assert(base->value == 21);
    ShrdPtr<Tracked> copy = base;
    assert(base.UseCount() == 2);
}

void TestConvertEmpty() {
    ShrdPtr<Derived> empty;
    ShrdPtr<Tracked> converted = empty;
    assert(!converted);
    assert(converted.UseCount() == 0);
}

void TestConvertingAssign() {
    ShrdPtr<Tracked> target{UnqPtr<Tracked>(new Tracked(1))};
    ShrdPtr<Derived> source{UnqPtr<Derived>(new Derived(2))};
    assert(Tracked::alive == 2);
    target = source;
    assert(Tracked::alive == 1);
    assert(target->value == 2);
    source = ShrdPtr<Derived>();
    assert(Tracked::alive == 1);
    target = ShrdPtr<Tracked>();
    assert(Tracked::alive == 0);
}

void TestMultipleInheritanceNoVirtual() {
    {
        ShrdPtr<PlainC> c{UnqPtr<PlainC>(new PlainC)};
        ShrdPtr<PlainB> b = c;
        ShrdPtr<PlainA> a = c;
        assert(c.UseCount() == 3);
        assert(a->a == 1);
        assert(b->b == 2);
        c = ShrdPtr<PlainC>();
        a = ShrdPtr<PlainA>();
        assert(PlainC::destroyed == 0);
    }
    assert(PlainC::destroyed == 1);
}

void TestChainNoVirtual() {
    {
        ShrdPtr<PlainMid> mid{UnqPtr<PlainLeaf>(new PlainLeaf)};
        ShrdPtr<PlainA> base = mid;
        assert(mid.UseCount() == 2);
        assert(base.UseCount() == 1);
        assert(base->a == 1);
    }
    assert(PlainLeaf::destroyed == 1);
}

void TestBaseFromNewNoVirtual() {
    {
        ShrdPtr<PlainB> b{UnqPtr<PlainB>(new PlainC)};
        ShrdPtr<PlainB> copy = b;
        assert(b.UseCount() == 2);
        assert(copy->b == 2);
    }
    assert(PlainC::destroyed == 1);
    assert(PlainA::destroyed == 1);
}

void TestNoLeaks() {
    for (int i = 0; i < 1000; ++i) {
        ShrdPtr<Tracked> a{UnqPtr<Tracked>(new Tracked(i))};
        ShrdPtr<Tracked> b = a;
        ShrdPtr<Tracked> c = b;
        ShrdPtr<Tracked> fromDerived = ShrdPtr<Derived>(UnqPtr<Derived>(new Derived(i)));
    }
    assert(Tracked::alive == 0);
}

}

void RunShrdPtrTests() {
    std::cout << "ShrdPtr\n";
    RunTest("счётчик ссылок", TestCount);
    RunTest("присваивание", TestAssign);
    RunTest("самоприсваивание", TestSelfAssign);
    RunTest("перемещение", TestMove);
    RunTest("пустой указатель", TestEmpty);
    RunTest("подтипизация Derived -> Base", TestSubtyping);
    RunTest("дескриптор базы переживает наследника", TestBaseOutlivesDerived);
    RunTest("приведение пустого указателя", TestConvertEmpty);
    RunTest("конвертирующее присваивание", TestConvertingAssign);
    RunTest("множественное наследование без virtual", TestMultipleInheritanceNoVirtual);
    RunTest("цепочка Leaf -> Mid -> Base без virtual", TestChainNoVirtual);
    RunTest("указатель на вторую базу сразу из new без virtual", TestBaseFromNewNoVirtual);
    RunTest("1000 циклов без утечек", TestNoLeaks);
}