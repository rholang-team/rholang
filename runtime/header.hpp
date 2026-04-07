#pragma once

#include <cstddef>

namespace memory_manager {
struct alignas(std::max_align_t) Header {
    union {
        Header* next;
        void* ref_map;
    };
    size_t size;     // cell size of a bin or 0 for large bin.
    bool mark;       // TODO move into size
    bool allocated;  // TODO move into size
};

struct alignas(std::max_align_t) MapHeader {
    void* start;
    void* end;
    MapHeader* next;
};

}  // namespace memory_manager

static constexpr size_t MIN_ALLOCATION_SIZE = sizeof(void*);
