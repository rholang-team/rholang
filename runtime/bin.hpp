#pragma once

#include <sys/mman.h>
#include <unistd.h>

#include <cstddef>

#include "header.hpp"
#include "page_size.hpp"

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

    template <typename F>
    void foreach_cell(F&& visitor) {
        for (MapHeader* r = map_head; r != nullptr; r = r->next) {
            void* cur = r->start;
            while (cur != (char*)r + PAGE_SIZE) {
                visitor((Header*)cur);
                cur = (char*)cur + sizeof(Header) + class_size;
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

private:
    void extend();
    void init_mapping(void* page);
};

}  // namespace memory_manager::alloc
