#include <stack>

#include "alloc.hpp"

#define CLEAR_INFO_BITS(p) p &= 0x0000ffffffffffff;
#define MARK_OBJ(p) p |= (1 << 63)
#define UNMARK_OBJ(p) p &= 0x7fffffffffffffff;

namespace GC {

class GC {
    struct Object {
        // 62 bit - mark bit.
        // 61 - 48 are reserved.
        void* ref_map;
    };

    Alloc allocator;
    std::stack<void*> mark_stack;

public:
    void* allocate(size_t size, void* ref_map);
    void safepoint();

private:
    void mark_from_roots();
    void mark();
    void sweep();
};

}  // namespace GC
