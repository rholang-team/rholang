#pragma once

#include <array>
#include <cstddef>
#include <cstdlib>

#include "bin.hpp"
#include "fla.hpp"

namespace memory_manager::alloc {

class MainAllocator {
    static constexpr size_t BIN_LIMIT = 8;
    static constexpr size_t LARGE_BIN = BIN_LIMIT + 1;

    std::array<Bin, BIN_LIMIT> bins;
    FreeListAllocator large_bin;

public:
    MainAllocator();

    void* allocate(size_t size);
    void deallocate(void* p);

private:
    /// returns an appropriate bin index for the size, LARGE_BIN if there is no appropriate bins.
    inline static size_t get_bin(size_t size);
    /// returns the bin where p resides.
    inline static size_t get_bin(void* p);
};
}  // namespace memory_manager::alloc
