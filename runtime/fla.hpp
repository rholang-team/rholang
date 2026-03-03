#pragma once

#include <cstddef>

#include "header.hpp"

namespace memory_manager::alloc {

class FreeListAllocator {
    Header* free_head;
    MapHeader* map_head;

public:
    FreeListAllocator();

    void* allocate(size_t size);
    void deallocate(void* p);

    template <typename F>
    void foreach_cell(F&& visitor);
    template <typename F>
    void foreach_allocated(F&& visitor);

private:
    void extend(size_t required_size);
    Header* find(size_t size, Header** prev) const;
    void split_if_possible(Header* cell, size_t needed_size);
    Header* insert(Header* cell);
    void coalesce_with_next(Header* cell);
};

}  // namespace memory_manager::alloc
