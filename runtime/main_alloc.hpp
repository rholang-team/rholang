#pragma once

#include <array>
#include <cstddef>
#include <cstdlib>

#include "bin.hpp"
#include "fla.hpp"

namespace memory_manager::alloc {

static constexpr size_t BIN_LIMIT = 8;
static constexpr size_t LARGE_BIN = BIN_LIMIT + 1;

class MainAllocator {
public:
    std::array<Bin, BIN_LIMIT> bins;
    FreeListAllocator large_bin;

    MainAllocator();

    void* allocate(size_t size);
    void deallocate(void* p);

    template <typename F>
    void foreach_cell(F&& visitor) {
        for (auto& bin : bins) {
            bin.foreach_cell(visitor);
        }
        large_bin.foreach_cell(visitor);
    }

    template <typename F>
    void foreach_allocated(F&& visitor) {
        for (auto& bin : bins) {
            bin.foreach_allocated(visitor);
        }
        large_bin.foreach_allocated(visitor);
    }

    bool empty();

private:
    /// returns an appropriate bin index for the size, LARGE_BIN if there is no
    /// appropriate bins.
    inline static size_t get_bin(size_t size);
    /// returns the bin where p resides.
    inline static size_t get_bin(void* p);
};
}  // namespace memory_manager::alloc
