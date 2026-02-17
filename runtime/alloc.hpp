#pragma once

#include <cstddef>
#include <cstdlib>

namespace GC {

class Alloc {
public:
    Alloc(size_t size);
    ~Alloc();

    void* allocate(size_t size);
    void deallocate(void* p);

private:
    struct alignas(std::max_align_t) Header {
        Header* next;
        size_t size;  // header + payload
    };

    void* heap;
    size_t heap_size;
    Header* head;
    static constexpr size_t MIN_ALLOCATION_SIZE = sizeof(void*);

    Header* find(size_t size, Header** prev) const;
    void split_if_possible(Header* block, size_t needed_size);
    Header* insert(Header* block);
    void coalesce_with_next(Header* block);
};
}  // namespace GC
