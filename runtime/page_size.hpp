#pragma once

#include <unistd.h>
#define PAGE_SIZE page_size

namespace memory_manager::alloc {
extern size_t page_size;
}  // namespace memory_manager::alloc
