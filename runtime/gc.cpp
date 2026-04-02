#include "gc.hpp"

#include "header.hpp"

namespace memory_manager {
void* GC::allocate(size_t size, void* ref_map) {
    auto p = allocator.allocate(size);
    Header* h = (Header*)((char*)p - sizeof(Header));
    h->ref_map = ref_map;
    h->mark = false;

    return p;
}

void GC::collect() {
    scan();
    mark();
    sweep();
}

// scans the roots and pushes them onto mark stack.
void GC::scan() {
    for (auto frame : shadow_stack) {
        auto n = frame->n_roots;
        auto roots = (void**)((char*)frame + sizeof(unsigned short));
        for (size_t i = 0; i < n; ++i) {
            mark_stack.push_back(roots[i]);
        }
    }
}

void GC::mark() {
    while (!mark_stack.empty()) {
        Header* obj = static_cast<Header*>(mark_stack.back());
        mark_stack.pop_back();
        if (obj->mark) {
            continue;
        } else {
            obj->mark = true;
            auto base = (void**)((char*)obj + sizeof(Header));
            auto rmap = (RefMap*)obj->ref_map;
            auto slots = rmap->n_slots;
            auto bmap = (unsigned char*)rmap + sizeof(size_t);

            for (size_t slot = 0; slot < slots; ++slot) {
                auto byte = bmap[slot / sizeof(void*)];
                if ((byte >> (slot % sizeof(unsigned char))) & 1) {
                    mark_stack.push_back(base + slot);
                }
            }
        }
    }
}

void GC::sweep() {
    allocator.foreach_cell([this](Header* cell) {
        if (!cell->mark) {
            allocator.deallocate((char*)cell + sizeof(Header));
        } else {
            cell->mark = false;
        }
    });
}

}  // namespace memory_manager
