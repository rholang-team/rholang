#include "bin.hpp"

#include <gtest/gtest.h>

#include <random>
#include <vector>

namespace memory_manager::alloc {

static constexpr size_t next_pow2(size_t n) {
    if (n <= 1)
        return 1;
    size_t p = 1;
    while (p < n)
        p <<= 1;
    return p;
}

struct TestStruct {
    char a;
    short b;
    size_t c;
};

TEST(AllocBinTest, ValuedLinearAllocation) {
    Bin bin(next_pow2(sizeof(TestStruct)));

    TestStruct* a = (TestStruct*)bin.allocate();
    ASSERT_NE(a, nullptr);
    a->a = 'a';
    a->b = 12;
    a->c = 0xbebebebe;

    TestStruct* b = (TestStruct*)bin.allocate();
    ASSERT_NE(b, nullptr);
    b->a = 'b';
    b->b = 13;
    b->c = 0xbebebebe + 1;

    TestStruct* c = (TestStruct*)bin.allocate();
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

    bin.deallocate(a);
    bin.deallocate(b);
    bin.deallocate(c);
}

TEST(AllocBinTest, HugeLinearAllocation) {
    constexpr size_t size = 256;
    Bin bin(next_pow2(size));

    std::vector<void*> objs;
    for (int i = 0; i < 128; i++) {
        objs.push_back(bin.allocate());
    }
    for (int i = 0; i < 128; i++) {
        bin.deallocate(objs.back());
        objs.pop_back();
    }
}

TEST(AllocBinTest, FuzzAllocDeallocOrder) {
    constexpr size_t size_threshold = 29;
    constexpr size_t heap_limit = 1900;

    Bin bin(next_pow2(size_threshold));
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dist(1, size_threshold);
    std::vector<void*> objs;

    for (size_t total = 0; total < heap_limit;) {
        auto size = dist(g);
        total += size;
        objs.push_back(bin.allocate());
    }

    std::shuffle(objs.begin(), objs.end(), g);

    for (auto p : objs) {
        bin.deallocate(p);
    }
}
}  // namespace memory_manager::alloc
