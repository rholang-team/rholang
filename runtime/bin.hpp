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

public:
    Bin() = default;
    Bin(size_t class_sz);

    void* allocate();
    void deallocate(void* p);

private:
    void extend();
    void init_free_list(void* page, Header* end);
};

}  // namespace memory_manager::alloc
