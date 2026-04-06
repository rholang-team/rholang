#include "runtime/fla.hpp"

#include <gtest/gtest.h>

#include <random>
#include <vector>

namespace memory_manager::alloc {

struct TestStruct {
    char a;
    short b;
    size_t c;
};

TEST(AllocFLATest, ValuedLinearAllocation) {
    FreeListAllocator fla;

    TestStruct* a = (TestStruct*)fla.allocate(sizeof(TestStruct));
    ASSERT_NE(a, nullptr);
    a->a = 'a';
    a->b = 12;
    a->c = 0xbebebebe;

    TestStruct* b = (TestStruct*)fla.allocate(sizeof(TestStruct));
    ASSERT_NE(b, nullptr);
    b->a = 'b';
    b->b = 13;
    b->c = 0xbebebebe + 1;

    TestStruct* c = (TestStruct*)fla.allocate(sizeof(TestStruct));
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

    fla.deallocate(a);
    fla.deallocate(b);
    fla.deallocate(c);
}

TEST(AllocFLATest, HugeLinearAllocation) {
    constexpr size_t size = 256;
    FreeListAllocator fla;

    std::vector<void*> objs;
    for (int i = 0; i < 128; i++) {
        objs.push_back(fla.allocate(size));
    }
    for (int i = 0; i < 128; i++) {
        fla.deallocate(objs.back());
        objs.pop_back();
    }
}

TEST(AllocFLATest, FuzzAllocDeallocOrder) {
    constexpr size_t size_threshold = 7000;
    constexpr size_t heap_limit = 1 << 22;

    FreeListAllocator fla;
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dist(1, size_threshold);
    std::vector<void*> objs;

    for (size_t total = 0; total < heap_limit;) {
        auto size = dist(g);
        total += size;
        objs.push_back(fla.allocate(size));
    }

    std::shuffle(objs.begin(), objs.end(), g);

    for (auto p : objs) {
        fla.deallocate(p);
    }
}
}  // namespace memory_manager::alloc
