#include "runtime.hpp"

#include <atomic>
#include <cstddef>
#include <mutex>
#include <random>
#include <thread>

#include "header.hpp"

memory_manager::Runtime runtime;

extern "C" {
void runtime_alloc(size_t size, void* rmap) {
    runtime.allocate(size, rmap);
}
void runtime_push_frame(void* frame) {
    runtime.push_frame(static_cast<memory_manager::FrameMap*>(frame));
}
void runtime_pop_frame() {
    runtime.pop_frame();
}
void runtime_collect() {
    runtime.collect();
}
}

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

void Runtime::scan() {
    std::lock_guard<std::mutex> lock(global_queue_mutex);
    for (auto frame : shadow_stack) {
        auto n = frame->n_roots;
        for (size_t i = 0; i < n; ++i) {
            global_root_queue.push_back(frame->roots[i]);
        }
    }
}

void Runtime::mark() {
    active_workers.store(NUM_WORKERS, std::memory_order_relaxed);
    marking_done.store(false, std::memory_order_relaxed);

    std::vector<std::thread> threads;
    for (size_t i = 0; i < NUM_WORKERS; ++i)
        threads.emplace_back(&Runtime::mark_worker, this, i);
    for (auto& t : threads)
        t.join();
}

void Runtime::mark_worker(size_t worker_id) {
    auto& my_deque = local_deques[worker_id];
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dist(0, NUM_WORKERS - 1);

    while (!marking_done.load(std::memory_order_acquire)) {
        void* obj = nullptr;

        obj = my_deque.pop();

        if (!obj) {
            for (size_t attempt = 0; attempt < NUM_WORKERS; ++attempt) {
                size_t victim = dist(rng);
                obj = local_deques[victim].steal();
                if (obj)
                    break;
            }
        }

        if (!obj) {
            std::lock_guard<std::mutex> lk(global_queue_mutex);
            if (!global_root_queue.empty()) {
                obj = global_root_queue.back();
                global_root_queue.pop_back();
            }
        }

        if (obj) {
            Header* hdr = (Header*)((char*)obj - sizeof(Header));
            bool expected = false;
            if (hdr->mark.compare_exchange_strong(expected,
                                                  true,
                                                  std::memory_order_acq_rel)) {
                auto base = (void**)((char*)hdr + sizeof(Header));
                auto rmap = (RefMap*)hdr->ref_map;
                if (rmap) {
                    for (size_t slot = 0; slot < rmap->n_slots; ++slot) {
                        auto byte = rmap->bmap[slot / 8];
                        if ((byte >> (slot % 8)) & 1) {
                            void* child = *(base + slot);
                            if (child) {
                                while (!my_deque.push(child)) {
                                    std::lock_guard<std::mutex> lk(
                                        global_queue_mutex);
                                    global_root_queue.push_back(child);
                                }
                            }
                        }
                    }
                }
            }
            continue;
        }

        std::size_t prev =
            active_workers.fetch_sub(1, std::memory_order_relaxed);
        if (prev == 1) {
            bool really_empty = true;

            {
                std::lock_guard<std::mutex> lk(global_queue_mutex);
                if (!global_root_queue.empty())
                    really_empty = false;
            }

            for (auto& d : local_deques) {
                if (!d.empty()) {
                    really_empty = false;
                    break;
                }
            }

            if (really_empty) {
                marking_done.store(true, std::memory_order_release);
                done_cv.notify_all();
                return;
            } else {
                active_workers.fetch_add(1, std::memory_order_relaxed);
                continue;
            }
        }

        std::unique_lock<std::mutex> lk(done_mutex);
        done_cv.wait_for(lk, std::chrono::microseconds(10), [this] {
            return marking_done.load(std::memory_order_acquire);
        });
        if (marking_done.load(std::memory_order_acquire))
            return;

        active_workers.fetch_add(1, std::memory_order_relaxed);
    }
}

void Runtime::sweep() {
    auto visitor = [this](Header* cell) {
        if (!cell->mark) {
            allocator.deallocate((char*)cell + sizeof(Header));
        } else {
            cell->mark.store(false, std::memory_order_relaxed);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(alloc::BIN_LIMIT + 1);

    for (size_t i = 0; i < alloc::BIN_LIMIT; ++i) {
        threads.emplace_back([this, i, visitor]() {
            allocator.bins[i].foreach_allocated(visitor);
        });
    }

    threads.emplace_back(
        [this, visitor]() { allocator.large_bin.foreach_allocated(visitor); });

    for (auto& t : threads) {
        t.join();
    }
}

bool Runtime::empty() {
    return allocator.empty();
}
}  // namespace memory_manager
