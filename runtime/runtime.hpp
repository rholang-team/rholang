#pragma once

#include <deque>

#include "main_alloc.hpp"

namespace memory_manager {

struct FrameMap {
    size_t n_roots;
    void* roots[];
};

class Runtime {
    struct RefMap {
        size_t n_slots;
        unsigned char bmap[];
    };
    alloc::MainAllocator allocator;
    std::deque<void*> mark_stack;
    std::deque<FrameMap*> shadow_stack;

public:
    Runtime() {}
    void* allocate(size_t size, void* ref_map);
    void collect();
    void push_frame(FrameMap* frame);
    void pop_frame();
    void scan();
    void mark();
    void sweep();
    bool empty();
};

}  // namespace memory_manager
