#include "bin.hpp"

#include <sys/mman.h>

#include "page_size.hpp"

namespace memory_manager::alloc {

inline size_t align_up(size_t n) {
    size_t align = alignof(std::max_align_t);
    return (n + (align - 1)) & ~(align - 1);
}

void Bin::init_free_list(void* page, Header* end) {
    Header* cur;

    for (size_t i = 0; i < PAGE_SIZE / entry_size - 1; i++) {
        cur = (Header*)((char*)page + i * entry_size);
        auto next = (Header*)((char*)cur + entry_size);
        cur->next = next;
        cur->mark = false;
        cur->allocated = false;
    }

    cur->next = end;
}

void Bin::extend() {
    size_t pages_needed = (entry_size > PAGE_SIZE) ? (entry_size / PAGE_SIZE) + 1 : 1;
    auto new_page = mmap(nullptr,
                         pages_needed * PAGE_SIZE,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS,
                         -1,
                         0);
    init_free_list(new_page, free_head);
    free_head = (Header*)new_page;
}

Bin::Bin(size_t class_sz) : class_size(class_sz) {
    entry_size = align_up(class_size + sizeof(Header));
    free_head = nullptr;
}

void* Bin::allocate() {
    if (!free_head) {
        extend();
    }

    auto p = free_head;
    p->allocated = true;
    p->size = class_size;
    free_head = p->next;

    return (char*)p + sizeof(Header);
}

void Bin::deallocate(void* p) {
    auto cell = (Header*)((char*)p - sizeof(Header));
    cell->allocated = false;
    cell->next = free_head;
    free_head = cell;
}

}  // namespace memory_manager::alloc
