#pragma once

#include <deque>

#include "main_alloc.hpp"

namespace memory_manager {

class GC {
    struct FrameMap {
        size_t n_roots;
        // there lie flat n_roots pointers to roots
    };

    struct RefMap {
        size_t n_slots;
        // there lie flat n_slots unsigned chars of (ptr size aligned) bitmap
    };

    alloc::MainAllocator allocator;
    std::deque<void*> mark_stack;
    std::deque<FrameMap*> shadow_stack;

public:
    GC() {}
    void* allocate(size_t size, void* ref_map);
    void collect();

private:
    void scan();
    void mark();
    void sweep();
};

}  // namespace memory_manager
