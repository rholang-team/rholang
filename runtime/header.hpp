#pragma once

#include <cstddef>

namespace memory_manager::alloc {
struct alignas(std::max_align_t) Header {
    Header* next;
    size_t size;  // cell size of a bin or 0 for large bin.
    bool mark;
    bool allocated;
};

struct alignas(std::max_align_t) MapHeader {
    void* start;
    void* end;
    MapHeader* next;
};

}  // namespace memory_manager::alloc

static constexpr size_t MIN_ALLOCATION_SIZE = sizeof(void*);
