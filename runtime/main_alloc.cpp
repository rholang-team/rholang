#include "main_alloc.hpp"

#include <bit>
#include <exception>
#include <iostream>

#include "header.hpp"
#include "page_size.hpp"

namespace memory_manager::alloc {

MainAllocator::MainAllocator() {
    auto cur_size = MIN_ALLOCATION_SIZE;
    for (size_t i = 0; i < BIN_LIMIT; i++) {
        bins[i] = Bin(cur_size);
        cur_size *= 2;
    }

    if (cur_size >= PAGE_SIZE - sizeof(MapHeader)) {
        std::cerr << "INSANE BIN LIMIT!\n";
        std::terminate();
    }
}

inline size_t MainAllocator::get_bin(size_t size) {
    auto cur_size = MIN_ALLOCATION_SIZE;
    for (size_t i = 0; i < BIN_LIMIT; i++) {
        if (size <= cur_size) {
            return i;
        }
        cur_size *= 2;
    }
    return LARGE_BIN;
}

inline size_t MainAllocator::get_bin(void* p) {
    auto header = (Header*)((char*)p - sizeof(Header));
    auto size = header->size;  // size field is always the cell size.

    if (size == 0 || size > (MIN_ALLOCATION_SIZE << (BIN_LIMIT - 1))) {
        return LARGE_BIN;
    }

    return std::countr_zero(size) - std::countr_zero(MIN_ALLOCATION_SIZE);
}

void* MainAllocator::allocate(size_t size) {
    auto idx = get_bin(size);
    if (idx == LARGE_BIN) {
        return large_bin.allocate(size);
    }

    return bins[idx].allocate();
}

void MainAllocator::deallocate(void* p) {
    auto idx = get_bin(p);
    if (idx == LARGE_BIN) {
        return large_bin.deallocate(p);
    }
    return bins[idx].deallocate(p);
}

bool MainAllocator::empty() {
    for (auto& bin : bins) {
        if (!bin.empty())
            return false;
    };
    return large_bin.empty();
}
}  // namespace memory_manager::alloc
