#include "gc.hpp"

#include "header.hpp"

namespace memory_manager {
void GC::collect() {
    scan_roots();
    mark();
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
            allocator.deallocate(cell);
        } else {
            cell->mark = false;
        }
    });
}

}  // namespace memory_manager
