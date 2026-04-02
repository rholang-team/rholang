#include "main_alloc.hpp"

#include <gtest/gtest.h>

#include <deque>
#include <random>
#include <vector>

namespace memory_manager::alloc {

struct TestStruct {
    char a;
    short b;
    size_t c;
};

TEST(AllocMainTest, ValuedLinearAllocation) {
    MainAllocator gpa;

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

TEST(AllocMainTest, HugeLinearAllocation) {
    constexpr size_t size = 256;
    MainAllocator gpa;

    std::vector<void*> objs;
    for (int i = 0; i < 128; i++) {
        objs.push_back(gpa.allocate(size));
    }
    for (int i = 0; i < 128; i++) {
        gpa.deallocate(objs.back());
        objs.pop_back();
    }
}

double pareto(double xm, double alpha, std::mt19937& gen) {
    std::uniform_real_distribution<double> dist(0.0, 1.1);
    return xm / std::pow(dist(gen), 1.0 / alpha);
}

TEST(DELIVERABLES__MainAlloc, Linear) {
    constexpr size_t size_threshold = 2048;
    constexpr size_t obj_limit = 10000;

    MainAllocator gpa;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<void*> objs;

    for (size_t i = 0; i < obj_limit; ++i) {
        auto size = size_threshold * pareto(1.0, 2.0, gen);
        objs.push_back(gpa.allocate(size));
    }

    for (auto p : objs) {
        gpa.deallocate(p);
    }
}

TEST(DELIVERABLES__MainAlloc, FuzzAllocDeallocOrder) {
    constexpr size_t size_threshold = 2048;
    constexpr size_t max_op = 20000;
    constexpr size_t max_pop = 10000;

    MainAllocator gpa;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> sized(0, size_threshold);
    std::uniform_int_distribution<int> op(0, 1);
    std::vector<void*> living;
    living.reserve(max_pop);

    for (size_t i = 0; i < max_op; ++i) {
        if (!living.empty() && op(gen)) {
            std::uniform_int_distribution<size_t> idx(0, living.size() - 1);
            auto i = idx(gen);
            gpa.deallocate(living[i]);
            living[i] = living.back();
            living.pop_back();
        } else {
            if (living.size() < max_pop) {
                living.push_back(gpa.allocate(sized(gen)));
            }
        }
    }

    for (auto p : living) {
        gpa.deallocate(p);
    }
}
}  // namespace memory_manager::alloc
