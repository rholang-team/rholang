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
    // scan_roots();
    // mark();
    sweep();
}

// scans the roots and pushes them onto mark stack.
void GC::scan_roots() {
    // TODO #3. also, to be discussed in #4
}

void GC::mark() {
    while (!mark_stack.empty()) {
        Header* obj = static_cast<Header*>(mark_stack.top());
        if (obj->mark) {
            continue;
        } else {
            obj->mark = true;
            // TODO get ref fields, push each into mark stack.
            // to be discussed in #4
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
