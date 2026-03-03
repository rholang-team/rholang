#pragma once

#include <sys/mman.h>
#include <unistd.h>

#include <cstddef>

#include "header.hpp"

namespace memory_manager::alloc {

class Bin {
    size_t entry_size;
    size_t class_size;
    Header* free_head;
    MapHeader* map_head;

public:
    Bin() = default;
    Bin(size_t class_sz);

    void* allocate();
    void deallocate(void* p);

private:
    void extend();
    void init_mapping(void* page);

    template <typename F>
    void foreach_cell(F&& visitor);
    template <typename F>
    void foreach_allocated(F&& visitor);
};

}  // namespace memory_manager::alloc
