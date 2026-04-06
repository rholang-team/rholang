#include "runtime/gc.hpp"

#include <gtest/gtest.h>
#include <math.h>
#include <sys/mman.h>

#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

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

    ASSERT_TRUE(gc.empty());
}

double pareto(double xm, double alpha, std::mt19937& gen) {
    std::uniform_real_distribution<double> dist(0.0, 1.1);
    return xm / std::pow(dist(gen), 1.0 / alpha);
}

TEST(DELIVERABLES__GC, SweepBig) {
    constexpr size_t size_threshold = 2048;
    constexpr size_t obj_limit = 10000;

    GC gc;
    std::random_device rd;
    std::mt19937 gen(rd());

    for (size_t i = 0; i < obj_limit; ++i) {
        auto size = size_threshold * pareto(1.0, 2.0, gen);
        gc.allocate(size, nullptr);
    }

    gc.collect();

    ASSERT_TRUE(gc.empty());
}

TEST(DELIVERABLES__GC, SweepFast) {
    constexpr size_t allocations_threshold = 100;
    constexpr size_t size_threshold = 2048;

    GC gc;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, allocations_threshold);

    for (size_t i = 0; i < 1000; ++i) {
        for (size_t j = 0; j < dist(gen); ++j) {
            auto size = size_threshold * pareto(1.0, 2.0, gen);
            gc.allocate(size, nullptr);
        }

        gc.collect();

        ASSERT_TRUE(gc.empty());
    }
    ASSERT_TRUE(gc.empty());
}

TEST(DELIVERABLES__GC, SweepFuzz) {
    constexpr size_t allocations_threshold = 50;
    constexpr size_t size_threshold = 2048;
    constexpr size_t live_limit = 100;

    GC gc;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, allocations_threshold);
    std::uniform_int_distribution<int> bd(0, 1);
    std::vector<void*> objs;

    for (size_t i = 0; i < 1000; ++i) {
        auto num = dist(gen);
        for (size_t j = 0; j < num; ++j) {
            auto size = size_threshold * pareto(1.0, 2.0, gen);
            objs.push_back(gc.allocate(size, nullptr));
        }

        std::shuffle(objs.begin(), objs.end(), gen);

        for (size_t j = 0; j < live_limit && j < num; ++j) {
            if (bd(gen)) {
                Header* h = (Header*)((char*)objs[j] - sizeof(Header));
                h->mark = true;
            }
        }

        gc.collect();
    }
    gc.collect();

    ASSERT_TRUE(gc.empty());
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
    ((char*)twin_ref_map)[8] = 0b00000011;

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
    ASSERT_LIVE(a);
    ASSERT_LIVE(b);
    ASSERT_LIVE(c);
    gc.sweep();
    ASSERT_DEAD(a);
    ASSERT_DEAD(b);
    ASSERT_DEAD(c);

    gc.pop_frame();

    gc.collect();
    ASSERT_TRUE(gc.empty());
}

TEST(DELIVERABLES__GC, MarkGallow) {
    GC gc;

    struct ExistentialCrisis {
        ExistentialCrisis* ever_avoidant_self;
    };

    void* ec_ref_map[2];
    ((size_t*)ec_ref_map)[0] = 1;
    ((char*)ec_ref_map)[8] = 0b00000001;

    ExistentialCrisis* c = (ExistentialCrisis*)gc.allocate(sizeof(ExistentialCrisis), ec_ref_map);
    c->ever_avoidant_self = c;
    ASSERT_DEAD(c);

    void* frame[2];
    frame[0] = (void*)1;
    frame[1] = c;

    gc.push_frame((FrameMap*)frame);

    gc.scan();
    gc.mark();
    ASSERT_LIVE(c);
    gc.sweep();
    ASSERT_DEAD(c);

    gc.pop_frame();
    gc.collect();
    ASSERT_TRUE(gc.empty());
}

TEST(DELIVERABLES__GC, MarkCyclicList) {
    GC gc;

    struct LiList {
        LiList* next;
    };

    void* ll_ref_map[2];
    ((size_t*)ll_ref_map)[0] = 1;
    ((char*)ll_ref_map)[8] = 0b00000001;

    LiList* a = (LiList*)gc.allocate(sizeof(LiList), ll_ref_map);
    LiList* b = (LiList*)gc.allocate(sizeof(LiList), ll_ref_map);
    LiList* c = (LiList*)gc.allocate(sizeof(LiList), ll_ref_map);
    LiList* d = (LiList*)gc.allocate(sizeof(LiList), ll_ref_map);
    a->next = b;
    b->next = c;
    c->next = a;
    d->next = a;
    ASSERT_DEAD(a);
    ASSERT_DEAD(b);
    ASSERT_DEAD(c);
    ASSERT_DEAD(d);

    void* frame[2];
    frame[0] = (void*)1;
    frame[1] = a;

    gc.push_frame((FrameMap*)frame);

    gc.scan();
    gc.mark();
    ASSERT_LIVE(a);
    ASSERT_LIVE(b);
    ASSERT_LIVE(c);
    ASSERT_DEAD(d);
    gc.sweep();
    ASSERT_DEAD(a);
    ASSERT_DEAD(b);
    ASSERT_DEAD(c);
    ASSERT_DEAD(d);

    gc.pop_frame();

    gc.collect();
    ASSERT_TRUE(gc.empty());
}

}  // namespace memory_manager
