#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>

#include "main_alloc.hpp"
#include "work_stealing_queue.hpp"

namespace memory_manager {

struct FrameMap {
    size_t n_roots;
    void* roots[];
};

class Runtime {
    struct RefMap {
        size_t n_slots;
        unsigned char bmap[];
    };

    static constexpr std::size_t NUM_WORKERS = 12;
    static constexpr std::size_t WORKLIST_CAP = 4096;

    alloc::MainAllocator allocator;
    std::deque<FrameMap*> shadow_stack;

    std::deque<void*> global_root_queue;
    std::mutex global_queue_mutex;

    std::array<WorkStealingDeque<WORKLIST_CAP>, NUM_WORKERS> local_deques;
    std::atomic<std::size_t> active_workers{0};
    std::atomic<bool> marking_done{true};
    std::condition_variable done_cv;
    std::mutex done_mutex;

public:
    Runtime() {};
    ~Runtime() {};
    void* allocate(size_t size, void* ref_map);
    void collect();
    void push_frame(FrameMap* frame);
    void pop_frame();
    void scan();
    void mark();
    void sweep();
    bool empty();

private:
    void mark_worker(std::size_t id);
};

}  // namespace memory_manager
