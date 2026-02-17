#include "alloc.hpp"

#include <cstddef>
#include <cstdlib>
#include <iostream>

namespace GC {

static inline size_t align_up(size_t n) {
    size_t align = alignof(std::max_align_t);
    return (n + (align - 1)) & ~(align - 1);
}

Alloc::Alloc(size_t size) {
    heap = std::malloc(size);
    heap_size = align_up(size);
    head = (Header*)heap;
    head->next = nullptr;
    head->size = heap_size;
}

Alloc::~Alloc() {
    std::free(heap);
}

void* Alloc::allocate(size_t size) {
    Header* prev;
    auto sz = size;
    if (size < MIN_ALLOCATION_SIZE) {
        sz = MIN_ALLOCATION_SIZE;
    }
    auto aligned_size = align_up(sz);
    auto block = find(aligned_size + sizeof(Header), &prev);

    if (!block) {
        std::cerr << "OOM\n";
        return nullptr;
    }

    split_if_possible(block, aligned_size);

    if (prev) {
        prev->next = block->next;
    } else {
        head = block->next;
    }

    return (char*)block + sizeof(Header);
}

void Alloc::deallocate(void* p) {
    auto block = (Header*)((char*)p - sizeof(Header));
    Header* prev = insert(block);
    coalesce_with_next(block);
    if (prev) {
        coalesce_with_next(prev);
    }
}

Alloc::Header* Alloc::find(size_t size, Header** prev) const {
    auto curr = head;
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

void Alloc::split_if_possible(Header* block, size_t payload_size) {
    auto total_size = payload_size + sizeof(Header);

    if (block->size > total_size) {
        auto new_block = (Header*)((char*)block + total_size);
        new_block->size = block->size - total_size;
        new_block->next = block->next;
        block->size = total_size;
        block->next = new_block;
    }
}

Alloc::Header* Alloc::insert(Header* block) {
    Header* curr = head;
    Header* prev = nullptr;

    while (curr != nullptr && curr < block) {
        prev = curr;
        curr = curr->next;
    }
    block->next = curr;

    if (prev) {
        prev->next = block;
    } else {
        head = block;
    }

    return prev;
}

void Alloc::coalesce_with_next(Header* block) {
    if (block && block->next) {
        if (((char*)block + block->size) == (char*)block->next) {
            block->size += block->next->size;
            block->next = block->next->next;
        }
    }
}
}  // namespace GC
