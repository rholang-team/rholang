#include <atomic>
#include <memory>

template <int CAPACITY>
class WorkStealingDeque {
    static_assert((CAPACITY & (CAPACITY - 1)) == 0,
                  "Capacity must be power of two");
    std::atomic<int> bottom{0}, top{0};  // signity is important!
    std::array<std::atomic<void*>, CAPACITY> buffer;

public:
    bool push(void* item) {
        int b = bottom.load(std::memory_order_relaxed);
        int t = top.load(std::memory_order_acquire);
        if (b - t >= CAPACITY - 1)
            return false;
        buffer[b & (CAPACITY - 1)].store(item, std::memory_order_relaxed);
        bottom.store(b + 1, std::memory_order_release);
        return true;
    }

    void* pop() {
        int b = bottom.load(std::memory_order_relaxed) - 1;
        bottom.store(b, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        int t = top.load(std::memory_order_relaxed);
        if (t <= b) {
            void* item =
                buffer[b & (CAPACITY - 1)].load(std::memory_order_relaxed);
            if (t < b)
                return item;
            if (!top.compare_exchange_strong(t,
                                             t + 1,
                                             std::memory_order_seq_cst,
                                             std::memory_order_relaxed))
                item = nullptr;
            bottom.store(t + 1, std::memory_order_relaxed);
            return item;
        } else {
            bottom.store(t, std::memory_order_relaxed);
            return nullptr;
        }
    }

    void* steal() {
        int t = top.load(std::memory_order_acquire);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        int b = bottom.load(std::memory_order_acquire);
        if (t < b) {
            void* item =
                buffer[t & (CAPACITY - 1)].load(std::memory_order_relaxed);
            if (!top.compare_exchange_strong(t,
                                             t + 1,
                                             std::memory_order_seq_cst,
                                             std::memory_order_relaxed))
                return nullptr;
            return item;
        }
        return nullptr;
    }

    bool empty() const {
        return top.load(std::memory_order_relaxed) ==
               bottom.load(std::memory_order_relaxed);
    }
};
