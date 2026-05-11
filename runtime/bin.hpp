#pragma once

#include <sys/mman.h>
#include <unistd.h>

#include <cstddef>

#include "header.hpp"

namespace memory_manager::alloc {

class alignas(64) Bin {
    size_t entry_size;
    size_t class_size;
    Header* free_head;
    MapHeader* map_head;

public:
    Bin() = default;
    Bin(size_t class_sz);

    void* allocate();
    void deallocate(void* p);

    template <typename F>
    void foreach_cell(F&& visitor) {
        for (MapHeader* r = map_head; r != nullptr; r = r->next) {
            void* cur = r->start;
            while ((char*)cur + entry_size <= (char*)r->end) {
                visitor((Header*)cur);
                cur = (char*)cur + entry_size;
            }
        }
    }

    template <typename F>
    void foreach_allocated(F&& visitor) {
        foreach_cell([&visitor](Header* cell) {
            if (cell->allocated) {
                visitor(cell);
            }
        });
    }

    bool empty();

private:
    void extend();
    void init_mapping(void* page);
};

}  // namespace memory_manager::alloc
