// ============================================================
// Atomics and the Memory Model (an Introduction)
// ============================================================
// std::atomic<T> makes single operations on T indivisible and
// visible to other threads without a mutex. For a counter or a
// flag it is simpler and usually faster than a lock. Equivalent
// to Java's AtomicInteger / AtomicBoolean.
//
// Note: C++ "volatile" is NOT for threads (unlike Java's volatile).
// It only stops the compiler optimising away accesses to memory-
// mapped hardware. For thread communication use std::atomic.
//
// Build and run:
//   cmake --build build --target concurrency_04_atomics
//   ./build/bin/concurrency_04_atomics
// ============================================================

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

constexpr int kThreads = 8;
constexpr int kIncrements = 100'000;

// ------------------------------------------------------------
// An Atomic Counter
// ------------------------------------------------------------
// ++counter on a std::atomic<int> is a single atomic
// read-modify-write (a "lock add" on x86). No lost updates.

void atomic_counter()
{
    std::cout << "=== Atomic Counter ===\n";
    std::atomic<long> counter{0};

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&counter] {
            for (int i = 0; i < kIncrements; ++i) {
                counter.fetch_add(1, std::memory_order_relaxed);   // see "Memory Orders" below
            }
        });
    }
    for (auto& th : threads) th.join();

    std::cout << "  expected " << kThreads * kIncrements << ", got " << counter.load() << '\n';
    std::cout << "  atomic<long> lock-free here? " << std::boolalpha << counter.is_lock_free() << '\n';
}

// ------------------------------------------------------------
// compare_exchange: Atomic "Update If Unchanged"
// ------------------------------------------------------------
// The building block of lock-free algorithms. Read the value,
// compute a new one, and store it ONLY if nobody changed the value
// in between; otherwise "expected" is refreshed and we retry.
// Here: keep the maximum value seen by any thread.

void atomic_max(std::atomic<int>& target, int value)
{
    int current = target.load();
    while (current < value && !target.compare_exchange_weak(current, value)) {
        // compare_exchange_weak updated "current" with the latest value; loop and retry
    }
}

void compare_exchange_demo()
{
    std::cout << "\n=== compare_exchange ===\n";
    std::atomic<int> highest{0};
    std::vector<std::thread> threads;
    for (int t = 1; t <= kThreads; ++t) {
        threads.emplace_back([&highest, t] {
            for (int i = 0; i < 1000; ++i) atomic_max(highest, t * 1000 + i);
        });
    }
    for (auto& th : threads) th.join();
    std::cout << "  highest value seen: " << highest.load() << " (expected " << kThreads * 1000 + 999 << ")\n";
}

// ------------------------------------------------------------
// Memory Orders
// ------------------------------------------------------------
// Every atomic operation takes an optional memory order:
//   seq_cst (default)  one global order everyone agrees on. Use it
//                      unless you have measured a need otherwise.
//   acquire / release  pairs: everything written BEFORE a release
//                      store is visible to a thread AFTER its
//                      acquire load of that same atomic. The
//                      standard "publish data, then set a flag" pattern.
//   relaxed            atomicity only, no ordering. Fine for a
//                      statistics counter nobody synchronises on.

struct Message {
    int id = 0;
    double payload = 0;
};

void publish_with_release_acquire()
{
    std::cout << "\n=== Publishing Data with release / acquire ===\n";

    Message message;                     // plain, non-atomic data
    std::atomic<bool> ready{false};

    std::thread producer{[&] {
        message.id = 7;                  // 1. write the data
        message.payload = 3.5;
        ready.store(true, std::memory_order_release);   // 2. publish
    }};

    std::thread consumer{[&] {
        while (!ready.load(std::memory_order_acquire)) {   // 3. wait for the flag
            std::this_thread::yield();
        }
        // 4. guaranteed to see the writes made before the release store
        std::cout << "  consumer read message " << message.id << " with payload " << message.payload << '\n';
    }};

    producer.join();
    consumer.join();
}

// ------------------------------------------------------------
// atomic_flag: the Minimal Lock-Free Type
// ------------------------------------------------------------
// The only type GUARANTEED lock-free on every platform. Enough to
// build a spinlock, shown for understanding only: a spinlock burns
// CPU while waiting and is rarely better than std::mutex outside
// very short critical sections.

class SpinLock {
public:
    void lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            flag_.wait(true, std::memory_order_relaxed);   // C++20: sleep until it changes
        }
    }
    void unlock()
    {
        flag_.clear(std::memory_order_release);
        flag_.notify_one();                                // C++20
    }

private:
    std::atomic_flag flag_;   // C++20: default-initialised to clear
};

void spinlock_demo()
{
    std::cout << "\n=== SpinLock from atomic_flag ===\n";
    SpinLock spin;
    long total = 0;                      // protected by spin
    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&] {
            for (int i = 0; i < 10'000; ++i) {
                spin.lock();
                ++total;
                spin.unlock();
            }
        });
    }
    for (auto& th : threads) th.join();
    std::cout << "  total = " << total << " (expected 40000)\n";
}

int main()
{
    atomic_counter();
    compare_exchange_demo();
    publish_with_release_acquire();
    spinlock_demo();
}
