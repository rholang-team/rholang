#pragma once

#include <atomic>
#include <cstddef>

namespace memory_manager {
struct alignas(std::max_align_t) Header {
    union {
        Header* next;
        void* ref_map;
    };
    size_t size;             // cell size of a bin or 0 for large bin.
    std::atomic<bool> mark;  // TODO move into size
    bool allocated;          // TODO move into size

    // helper functions for use in single threaded contexts only!
    /// RELAXED
    void set_mark() {
        mark.store(true, std::memory_order_relaxed);
    }

    /// RELAXED
    void clear_mark() {
        mark.store(false, std::memory_order_relaxed);
    }
};

struct alignas(std::max_align_t) MapHeader {
    void* start;
    void* end;
    MapHeader* next;
};

}  // namespace memory_manager

static constexpr size_t MIN_ALLOCATION_SIZE = sizeof(void*);
