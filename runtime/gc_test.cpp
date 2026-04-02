#include "gc.hpp"

#include <gtest/gtest.h>
#include <math.h>
#include <sys/mman.h>

#include <cstddef>
#include <random>

namespace memory_manager {

struct TestStruct {
    char a;
    short b;
    size_t c;
};

TEST(GCTest, ValuedLinearAllocation) {
    GC gc;

    TestStruct* a = (TestStruct*)gc.allocate(sizeof(TestStruct), nullptr);
    ASSERT_NE(a, nullptr);
    a->a = 'a';
    a->b = 12;
    a->c = 0xbebebebe;

    TestStruct* b = (TestStruct*)gc.allocate(sizeof(TestStruct), nullptr);
    ASSERT_NE(b, nullptr);
    b->a = 'b';
    b->b = 13;
    b->c = 0xbebebebe + 1;

    TestStruct* c = (TestStruct*)gc.allocate(sizeof(TestStruct), nullptr);
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

    gc.collect();
}

double pareto(double xm, double alpha, std::mt19937& gen) {
    std::uniform_real_distribution<double> dist(0.0, 1.1);
    return xm / std::pow(dist(gen), 1.0 / alpha);
}

TEST(GCTest, FuzzAllocCollect) {
    constexpr size_t size_threshold = 9000;
    constexpr size_t heap_limit = 1 << 25;

    GC gc;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<void*> objs;

    for (size_t total = 0; total < heap_limit;) {
        auto size = size_threshold * pareto(1.0, 2.0, gen);
        total += size;
        objs.push_back(gc.allocate(size, nullptr));
    }

    gc.collect();
}

#define ASSERT_LIVE(obj) ASSERT_TRUE(((Header*)((char*)obj - sizeof(Header)))->mark);
#define ASSERT_DEAD(obj) ASSERT_FALSE(((Header*)((char*)obj - sizeof(Header)))->mark);

TEST(DELIVERABLES__GC, MarkTrees) {
    GC gc;

    struct Twins {
        int* a;
        int* b;
    };

    void* twin_ref_map[2];
    ((size_t*)twin_ref_map)[0] = 2;
    ((char*)twin_ref_map)[9] = 0b11000000;

    int* a = (int*)gc.allocate(sizeof(int), nullptr);
    int* b = (int*)gc.allocate(sizeof(int), nullptr);
    *a = 0xaaaaaaaa;
    *b = 0xbbbbbbbb;
    ASSERT_DEAD(a);
    ASSERT_DEAD(b);

    Twins* c = (Twins*)gc.allocate(sizeof(Twins), twin_ref_map);
    c->a = a;
    c->b = b;
    ASSERT_DEAD(c);

    void* frame[2];
    frame[0] = (void*)1;
    frame[1] = c;

    gc.push_frame((FrameMap*)frame);

    gc.scan();
    gc.mark();

    gc.pop_frame();
}
}  // namespace memory_manager
