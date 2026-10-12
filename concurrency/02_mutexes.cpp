// ============================================================
// Mutexes and Locks
// ============================================================
// When two threads touch the same data and at least one writes,
// access must be synchronised. Without it you have a DATA RACE,
// which in C++ is undefined behaviour (not just a wrong count as
// in Java; see ub_data_race.cpp, an opt-in target).
//
//   std::mutex         the lock itself
//   std::lock_guard    RAII: locks in the constructor, unlocks in the destructor
//   std::unique_lock   like lock_guard, but movable and can unlock early
//   std::scoped_lock   C++17: locks SEVERAL mutexes without deadlock
//   std::shared_mutex  C++17: many readers OR one writer
//
// Never call mutex.lock() / unlock() by hand: an exception or
// early return between them leaves the mutex locked forever.
//
// Build and run:
//   cmake --build build --target concurrency_02_mutexes
//   ./build/bin/concurrency_02_mutexes
// ============================================================

#include <iostream>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

constexpr int kThreads = 8;
constexpr int kIncrements = 100'000;

// ------------------------------------------------------------
// lock_guard: Protecting a Counter
// ------------------------------------------------------------
// Keep the mutex next to the data it protects, ideally both
// private in one class, so no one can touch the data unlocked.

class SafeCounter {
public:
    void increment()
    {
        std::lock_guard lock{mutex_};   // CTAD: std::lock_guard<std::mutex>
        ++value_;
    }

    long value() const
    {
        std::lock_guard lock{mutex_};
        return value_;
    }

private:
    mutable std::mutex mutex_;   // mutable: locking in a const function is fine
    long value_ = 0;
};

void lock_guard_demo()
{
    std::cout << "=== lock_guard ===\n";
    SafeCounter counter;

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&counter] {
            for (int i = 0; i < kIncrements; ++i) counter.increment();
        });
    }
    for (auto& th : threads) th.join();

    std::cout << "  expected " << kThreads * kIncrements << ", got " << counter.value() << '\n';
}

// ------------------------------------------------------------
// scoped_lock: Several Mutexes Without Deadlock
// ------------------------------------------------------------
// Deadlock: thread 1 locks A then waits for B, thread 2 locks B
// then waits for A. Both wait forever. Locking in a consistent
// order prevents it; std::scoped_lock does it for you with a
// deadlock-avoidance algorithm, whatever order you list them in.

struct Account {
    std::string name;
    long balance;
    std::mutex mutex;
};

void transfer(Account& from, Account& to, long amount)
{
    std::scoped_lock lock{from.mutex, to.mutex};   // both or neither
    from.balance -= amount;
    to.balance += amount;
}

void scoped_lock_demo()
{
    std::cout << "\n=== scoped_lock: Transfers in Both Directions ===\n";
    Account alice{"Alice", 1000, {}};
    Account bob{"Bob", 1000, {}};

    // Opposite directions at the same time: the classic deadlock setup.
    std::thread t1{[&] { for (int i = 0; i < 10'000; ++i) transfer(alice, bob, 1); }};
    std::thread t2{[&] { for (int i = 0; i < 10'000; ++i) transfer(bob, alice, 1); }};
    t1.join();
    t2.join();

    std::cout << "  Alice " << alice.balance << ", Bob " << bob.balance
              << ", total " << alice.balance + bob.balance << " (no deadlock, no money lost)\n";
}

// ------------------------------------------------------------
// unique_lock: Unlocking Early
// ------------------------------------------------------------
// Hold a lock for as short a time as possible. unique_lock can
// unlock before the end of the scope, and is required by
// condition variables (03_condition_variable.cpp).

std::mutex g_log_mutex;
std::vector<std::string> g_log;

void log_and_work(const std::string& message)
{
    std::unique_lock lock{g_log_mutex};
    g_log.push_back(message);   // the shared part: under the lock
    lock.unlock();              // release before the slow, private work
    std::this_thread::yield();  // stands in for slow work that needs no lock
}

// ------------------------------------------------------------
// shared_mutex: Readers and Writers
// ------------------------------------------------------------
// Many threads may hold a shared_lock at once (readers); a
// unique_lock is exclusive (writer). Worth it when reads vastly
// outnumber writes.

class Config {
public:
    std::string get(const std::string& key) const
    {
        std::shared_lock lock{mutex_};   // shared: readers do not block each other
        auto it = values_.find(key);
        return it == values_.end() ? "" : it->second;
    }

    void set(const std::string& key, const std::string& value)
    {
        std::unique_lock lock{mutex_};   // exclusive
        values_[key] = value;
    }

private:
    mutable std::shared_mutex mutex_;
    std::map<std::string, std::string> values_;
};

int main()
{
    lock_guard_demo();
    scoped_lock_demo();

    std::cout << "\n=== unique_lock ===\n";
    std::thread a{log_and_work, "from thread a"};
    std::thread b{log_and_work, "from thread b"};
    a.join();
    b.join();
    std::cout << "  log has " << g_log.size() << " entries\n";

    std::cout << "\n=== shared_mutex ===\n";
    Config config;
    config.set("mode", "fast");
    std::vector<std::thread> readers;
    for (int i = 0; i < 4; ++i) {
        readers.emplace_back([&config] { (void)config.get("mode"); });
    }
    for (auto& r : readers) r.join();
    std::cout << "  mode = " << config.get("mode") << '\n';
}
