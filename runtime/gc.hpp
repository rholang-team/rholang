#pragma once

#include <stack>

#include "main_alloc.hpp"

namespace memory_manager {

class GC {
    alloc::MainAllocator allocator;
    std::stack<void*> mark_stack;

public:
    GC() {}
    void* allocate(size_t size, void* ref_map);
    void collect();

private:
    void scan_roots();
    void mark();
    void sweep();
};

}  // namespace memory_manager
