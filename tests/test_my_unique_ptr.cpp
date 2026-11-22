#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <utility>
#include "MyUniquePtr.h"

struct Tracker {
    static inline int alive_count = 0;
    int value{0};

    Tracker() : value(0) { ++alive_count; }
    explicit Tracker(int v) : value(v) { ++alive_count; }
    ~Tracker() { --alive_count; }
};

struct Base {
    static inline int base_destroyed = 0;
    virtual ~Base() { ++base_destroyed; }
    virtual int id() const { return 1; }
};

struct Derived : public Base {
    static inline int derived_destroyed = 0;
    ~Derived() override { ++derived_destroyed; }
    int id() const override { return 2; }
};

TEST(MyUniquePtrTraitsTest, NonCopyableAndMovable) {
    EXPECT_FALSE(std::is_copy_constructible_v<MyUniquePtr<int>>);
    EXPECT_FALSE(std::is_copy_assignable_v<MyUniquePtr<int>>);
    EXPECT_TRUE(std::is_move_constructible_v<MyUniquePtr<int>>);
    EXPECT_TRUE(std::is_move_assignable_v<MyUniquePtr<int>>);
}

TEST(MyUniquePtrConstructorTest, DefaultAndNullptrConstructor) {
    MyUniquePtr<int> p1;
    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_FALSE(static_cast<bool>(p1));

    MyUniquePtr<int> p2(nullptr);
    EXPECT_EQ(p2.get(), nullptr);
    EXPECT_FALSE(static_cast<bool>(p2));
}

TEST(MyUniquePtrConstructorTest, RawPointerConstructor) {
    EXPECT_EQ(Tracker::alive_count, 0);
    {
        MyUniquePtr<Tracker> ptr(new Tracker(42));
        EXPECT_EQ(Tracker::alive_count, 1);
        EXPECT_NE(ptr.get(), nullptr);
        EXPECT_TRUE(static_cast<bool>(ptr));
        EXPECT_EQ(ptr->value, 42);
    }
    EXPECT_EQ(Tracker::alive_count, 0);
}

TEST(MyUniquePtrMoveTest, MoveConstructor) {
    MyUniquePtr<Tracker> p1(new Tracker(10));
    EXPECT_EQ(Tracker::alive_count, 1);

    MyUniquePtr<Tracker> p2(std::move(p1));
    EXPECT_EQ(Tracker::alive_count, 1);
    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_FALSE(static_cast<bool>(p1));
    EXPECT_NE(p2.get(), nullptr);
    EXPECT_EQ(p2->value, 10);
}

TEST(MyUniquePtrMoveTest, MoveAssignment) {
    MyUniquePtr<Tracker> p1(new Tracker(10));
    MyUniquePtr<Tracker> p2(new Tracker(20));
    EXPECT_EQ(Tracker::alive_count, 2);

    p2 = std::move(p1);
    EXPECT_EQ(Tracker::alive_count, 1);
    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(p2->value, 10);
}

TEST(MyUniquePtrMoveTest, MoveAssignmentSelf) {
    MyUniquePtr<Tracker> p(new Tracker(55));
    p = std::move(p);
    EXPECT_EQ(Tracker::alive_count, 1);
    EXPECT_NE(p.get(), nullptr);
    EXPECT_EQ(p->value, 55);
}

TEST(MyUniquePtrMoveTest, NullptrAssignment) {
    MyUniquePtr<Tracker> p(new Tracker(77));
    EXPECT_EQ(Tracker::alive_count, 1);

    p = nullptr;
    EXPECT_EQ(Tracker::alive_count, 0);
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_FALSE(static_cast<bool>(p));
}

TEST(MyUniquePtrPolymorphismTest, ConvertingMoveConstructor) {
    Base::base_destroyed = 0;
    Derived::derived_destroyed = 0;

    {
        MyUniquePtr<Derived> derived = makeUnique<Derived>();
        MyUniquePtr<Base> base(std::move(derived));

        EXPECT_EQ(derived.get(), nullptr);
        EXPECT_NE(base.get(), nullptr);
        EXPECT_EQ(base->id(), 2);
    }

    EXPECT_EQ(Derived::derived_destroyed, 1);
    EXPECT_EQ(Base::base_destroyed, 1);
}

TEST(MyUniquePtrPolymorphismTest, ConvertingMoveAssignment) {
    Base::base_destroyed = 0;
    Derived::derived_destroyed = 0;

    {
        MyUniquePtr<Base> base = makeUnique<Base>();
        MyUniquePtr<Derived> derived = makeUnique<Derived>();

        base = std::move(derived);
        EXPECT_EQ(Base::base_destroyed, 1);
        EXPECT_EQ(derived.get(), nullptr);
        EXPECT_EQ(base->id(), 2);
    }

    EXPECT_EQ(Derived::derived_destroyed, 1);
    EXPECT_EQ(Base::base_destroyed, 2);
}

TEST(MyUniquePtrModifiersTest, Release) {
    Tracker* raw = nullptr;
    {
        MyUniquePtr<Tracker> p(new Tracker(99));
        raw = p.release();
        EXPECT_EQ(p.get(), nullptr);
        EXPECT_EQ(Tracker::alive_count, 1);
    }
    EXPECT_EQ(Tracker::alive_count, 1);
    EXPECT_EQ(raw->value, 99);
    delete raw;
    EXPECT_EQ(Tracker::alive_count, 0);

    MyUniquePtr<Tracker> empty;
    EXPECT_EQ(empty.release(), nullptr);
}

TEST(MyUniquePtrModifiersTest, Reset) {
    MyUniquePtr<Tracker> p(new Tracker(1));
    EXPECT_EQ(Tracker::alive_count, 1);

    p.reset(new Tracker(2));
    EXPECT_EQ(Tracker::alive_count, 1);
    EXPECT_EQ(p->value, 2);

    p.reset();
    EXPECT_EQ(Tracker::alive_count, 0);
    EXPECT_EQ(p.get(), nullptr);
}

TEST(MyUniquePtrModifiersTest, Swap) {
    MyUniquePtr<Tracker> p1(new Tracker(10));
    MyUniquePtr<Tracker> p2(new Tracker(20));

    p1.swap(p2);
    EXPECT_EQ(p1->value, 20);
    EXPECT_EQ(p2->value, 10);

    swap(p1, p2);
    EXPECT_EQ(p1->value, 10);
    EXPECT_EQ(p2->value, 20);
}

TEST(MyUniquePtrAccessTest, DereferenceAndArrow) {
    MyUniquePtr<std::string> p = makeUnique<std::string>("hello");
    EXPECT_EQ(*p, "hello");
    EXPECT_EQ(p->size(), 5);

    *p = "world";
    EXPECT_EQ(*p, "world");
}

TEST(MyUniquePtrFactoryTest, MakeUniqueWithArgs) {
    auto p = makeUnique<std::string>(4, 'x');
    EXPECT_NE(p.get(), nullptr);
    EXPECT_EQ(*p, "xxxx");
}

TEST(MyUniquePtrComparisonTest, EqualityOperators) {
    MyUniquePtr<int> p1 = makeUnique<int>(5);
    MyUniquePtr<int> p2 = makeUnique<int>(5);
    MyUniquePtr<int> empty1;
    MyUniquePtr<int> empty2;

    EXPECT_TRUE(p1 == p1);
    EXPECT_FALSE(p1 != p1);
    EXPECT_FALSE(p1 == p2);
    EXPECT_TRUE(p1 != p2);

    EXPECT_TRUE(empty1 == empty2);
    EXPECT_TRUE(empty1 == nullptr);
    EXPECT_FALSE(empty1 != nullptr);
    EXPECT_FALSE(p1 == nullptr);
    EXPECT_TRUE(p1 != nullptr);
}
