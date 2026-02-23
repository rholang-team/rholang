#include "fla.hpp"

#include <sys/mman.h>

#include "header.hpp"
#include "page_size.hpp"

namespace memory_manager::alloc {

// size of a free list's node is it's payload size.

inline size_t align_up(size_t n) {
    size_t align = alignof(std::max_align_t);
    return (n + (align - 1)) & ~(align - 1);
}

void FreeListAllocator::extend(size_t required_size) {
    size_t map_off = align_up(sizeof(MapHeader));
    size_t pages = (required_size + map_off + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t total = pages * PAGE_SIZE;
    auto new_page =
        mmap(nullptr, total, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    auto map = (MapHeader*)new_page;
    auto node = (Header*)((char*)new_page + map_off);
    map->start = node;
    map->end = (char*)map + total;

    if (map_head) [[likely]] {
        map->next = map_head->next;
    }

    map_head = map;

    node->size = total - sizeof(Header);
    node->allocated = false;
    node->mark = false;
    node->next = free_head;
    free_head = node;
}

FreeListAllocator::FreeListAllocator() : free_head(nullptr), map_head(nullptr) {}

void* FreeListAllocator::allocate(size_t size) {
    Header* prev;
    auto sz = size;
    if (size < MIN_ALLOCATION_SIZE) {
        sz = MIN_ALLOCATION_SIZE;
    }
    auto aligned_size = align_up(sz);
    auto cell = find(aligned_size, &prev);

    if (!cell) {
        extend(size + sizeof(Header));
        cell = find(aligned_size, &prev);
    }

    split_if_possible(cell, aligned_size);

    if (prev) {
        prev->next = cell->next;
    } else {
        free_head = cell->next;
    }

    cell->allocated = true;
    return (char*)cell + sizeof(Header);
}

void FreeListAllocator::deallocate(void* p) {
    auto cell = (Header*)((char*)p - sizeof(Header));
    Header* prev = insert(cell);
    cell->allocated = false;
    coalesce_with_next(cell);
    if (prev) {
        coalesce_with_next(prev);
    }
}

Header* FreeListAllocator::find(size_t size, Header** prev) const {
    auto curr = free_head;
    *prev = nullptr;

    while (curr != nullptr) {
        if (curr->size >= size) {
            return curr;
        } else {
            *prev = curr;
            curr = curr->next;
        }
    }

    return nullptr;
}

void FreeListAllocator::split_if_possible(Header* cell, size_t payload_size) {
    auto total_size = payload_size + sizeof(Header);

    if (cell->size >= total_size + sizeof(Header) + MIN_ALLOCATION_SIZE) {
        auto new_cell = (Header*)((char*)cell + total_size);
        new_cell->size = cell->size - total_size;
        new_cell->next = cell->next;
        new_cell->mark = false;
        new_cell->allocated = false;
        cell->size = payload_size;
        cell->next = new_cell;
    }
}

Header* FreeListAllocator::insert(Header* cell) {
    Header* curr = free_head;
    Header* prev = nullptr;

    while (curr != nullptr && curr < cell) {
        prev = curr;
        curr = curr->next;
    }
    cell->next = curr;

    if (prev) {
        prev->next = cell;
    } else {
        free_head = cell;
    }

    return prev;
}

void FreeListAllocator::coalesce_with_next(Header* cell) {
    if (cell && cell->next) {
        if (((char*)cell + sizeof(Header) + cell->size) == (char*)cell->next) {
            cell->size += sizeof(Header) + cell->next->size;
            cell->next = cell->next->next;
        }
    }
}

template <typename F>
void FreeListAllocator::foreach_cell(F&& visitor) {
    for (MapHeader* r = map_head; r != nullptr; r = r->next) {
        void* cur = r->start;
        void* end = r->end;

        while (cur < end) {
            Header* cell = (Header*)cur;
            if (cell->allocated) {
                visitor((Header*)((char*)cur + sizeof(Header)));
            }
            cur = (char*)cur + sizeof(Header) + cell->size;
        }
    }
}

template <typename F>
void FreeListAllocator::foreach_allocated(F&& visitor) {
    foreach_cell([&visitor](Header* cell) {
        if (cell->allocated) {
            visitor(cell);
        }
    });
}
}  // namespace memory_manager::alloc
