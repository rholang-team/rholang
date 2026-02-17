#include "alloc.hpp"

#include <gtest/gtest.h>

#include <random>
#include <vector>

struct TestStruct {
    char a;
    short b;
    size_t c;
};

TEST(AllocTest, ValuedLinearAllocation) {
    GC::Alloc gpa(128);

    TestStruct* a = (TestStruct*)gpa.allocate(sizeof(TestStruct));
    ASSERT_NE(a, nullptr);
    a->a = 'a';
    a->b = 12;
    a->c = 0xbebebebe;

    TestStruct* b = (TestStruct*)gpa.allocate(sizeof(TestStruct));
    ASSERT_NE(b, nullptr);
    b->a = 'b';
    b->b = 13;
    b->c = 0xbebebebe + 1;

    TestStruct* c = (TestStruct*)gpa.allocate(sizeof(TestStruct));
    ASSERT_NE(c, nullptr);
    c->a = 'c';
    c->b = 14;
    c->c = 0xbebebebe + 2;

    ASSERT_EQ(a->a, 'a');
    ASSERT_EQ(a->b, 12);
    ASSERT_EQ(a->c, 0xbebebebe);
    ASSERT_EQ(b->a, 'b');
    ASSERT_EQ(b->b, 13);
    ASSERT_EQ(b->c, 0xbebebebe + 1);
    ASSERT_EQ(c->a, 'c');
    ASSERT_EQ(c->b, 14);
    ASSERT_EQ(c->c, 0xbebebebe + 2);

    gpa.deallocate(a);
    gpa.deallocate(b);
    gpa.deallocate(c);
}

TEST(AllocTest, HugeLinearAllocation) {
    GC::Alloc gpa(32768 + 128 * 16);

    std::vector<void*> objs;
    for (int i = 0; i < 128; i++) {
        objs.push_back(gpa.allocate(256));
    }
    for (int i = 0; i < 128; i++) {
        gpa.deallocate(objs.back());
        objs.pop_back();
    }
}

TEST(AllocTest, FuzzAllocDeallocOrder) {
    constexpr size_t header_size = 16;
    constexpr size_t heap_size = 1 << 20;
    constexpr size_t size_threshold = 1024;

    GC::Alloc gpa(heap_size * header_size);
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dist(1, size_threshold);
    std::vector<void*> objs;

    for (size_t total = 0; total < heap_size;) {
        auto size = dist(g);
        total += size;
        objs.push_back(gpa.allocate(size));
    }

    std::shuffle(objs.begin(), objs.end(), g);

    for (auto p : objs) {
        gpa.deallocate(p);
    }
}
