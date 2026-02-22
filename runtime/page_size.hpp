#pragma once

#include <unistd.h>
#define PAGE_SIZE page_size()

namespace memory_manager::alloc {
inline size_t page_size() {
    static size_t pagesize = [] { return (size_t)sysconf(_SC_PAGESIZE); }();
    return pagesize;
}
}  // namespace memory_manager::alloc
