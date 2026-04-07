#include "runtime.hpp"

#include "header.hpp"

namespace memory_manager {
void* Runtime::allocate(size_t size, void* ref_map) {
    auto p = allocator.allocate(size);
    Header* h = (Header*)((char*)p - sizeof(Header));
    h->ref_map = ref_map;
    h->mark = false;

    return p;
}

void Runtime::collect() {
    scan();
    mark();
    sweep();
}

void Runtime::push_frame(FrameMap* frame) {
    shadow_stack.push_back(frame);
}

void Runtime::pop_frame() {
    shadow_stack.pop_back();
}

// scans the roots and pushes them onto mark stack.
void Runtime::scan() {
    for (auto frame : shadow_stack) {
        auto n = frame->n_roots;
        for (size_t i = 0; i < n; ++i) {
            mark_stack.push_back(frame->roots[i]);
        }
    }
}

void Runtime::mark() {
    while (!mark_stack.empty()) {
        Header* obj = (Header*)((char*)mark_stack.back() - sizeof(Header));
        mark_stack.pop_back();
        if (obj->mark) {
            continue;
        } else {
            obj->mark = true;
            auto base = (void**)((char*)obj + sizeof(Header));
            auto rmap = (RefMap*)obj->ref_map;
            if (!rmap)
                continue;

            for (size_t slot = 0; slot < rmap->n_slots; ++slot) {
                auto byte = rmap->bmap[slot / 8];
                if ((byte >> (slot % 8)) & 1) {
                    void* p = *(base + slot);
                    if (p) {
                        mark_stack.push_back(p);
                    }
                }
            }
        }
    }
}

void Runtime::sweep() {
    allocator.foreach_allocated([this](Header* cell) {
        if (!cell->mark) {
            allocator.deallocate((char*)cell + sizeof(Header));
        } else {
            cell->mark = false;
        }
    });
}

bool Runtime::empty() {
    return allocator.empty();
}
}  // namespace memory_manager
