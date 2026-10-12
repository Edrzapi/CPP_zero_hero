// ============================================================
// Condition Variables: Producer and Consumer
// ============================================================
// A condition variable lets a thread SLEEP until another thread
// says "something changed", instead of spinning in a loop. Always
// used with a mutex and a condition on shared state. Like Java's
// wait()/notify() and C#'s Monitor.Wait/Pulse.
//
// Rules:
//   - wait with a PREDICATE: wakeups can be spurious, and another
//     thread may have consumed the item first
//   - change the shared state while holding the mutex
//   - notify after changing it (holding the lock is optional)
//
// Build and run:
//   cmake --build build --target concurrency_03_condition_variable
//   ./build/bin/concurrency_03_condition_variable
// ============================================================

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// A Thread-Safe Blocking Queue
// ------------------------------------------------------------
// pop() blocks until an item arrives or the queue is closed.
// close() wakes every waiting consumer so they can exit.

template <typename T>
class BlockingQueue {
public:
    void push(T item)
    {
        {
            std::lock_guard lock{mutex_};
            items_.push(std::move(item));
        }
        ready_.notify_one();   // wake one waiting consumer
    }

    // Returns std::nullopt once the queue is closed and empty.
    std::optional<T> pop()
    {
        std::unique_lock lock{mutex_};   // wait() needs a unique_lock: it unlocks while sleeping
        ready_.wait(lock, [this] { return !items_.empty() || closed_; });
        if (items_.empty()) {
            return std::nullopt;         // closed and drained
        }
        T item = std::move(items_.front());
        items_.pop();
        return item;
    }

    void close()
    {
        {
            std::lock_guard lock{mutex_};
            closed_ = true;
        }
        ready_.notify_all();   // wake EVERY consumer
    }

private:
    std::mutex mutex_;
    std::condition_variable ready_;
    std::queue<T> items_;
    bool closed_ = false;
};

// ------------------------------------------------------------
// One Producer, Several Consumers
// ------------------------------------------------------------

int main()
{
    std::cout << "=== Producer / Consumer ===\n";

    BlockingQueue<int> jobs;
    std::mutex results_mutex;
    std::vector<std::string> results;

    auto consumer = [&](int id) {
        int handled = 0;
        while (auto job = jobs.pop()) {   // loop ends when pop returns nullopt
            ++handled;
            (void)*job;                    // a real worker would process the job here
            std::this_thread::sleep_for(std::chrono::milliseconds{1});   // simulated work spreads the jobs out
        }
        std::lock_guard lock{results_mutex};
        results.push_back("consumer " + std::to_string(id) + " handled " + std::to_string(handled));
    };

    std::vector<std::thread> consumers;
    for (int id = 1; id <= 3; ++id) {
        consumers.emplace_back(consumer, id);
    }

    for (int job = 1; job <= 30; ++job) {
        jobs.push(job);
    }
    jobs.close();   // no more work: consumers drain the queue and exit

    for (auto& c : consumers) c.join();

    int total = 0;
    for (const auto& line : results) {
        std::cout << "  " << line << '\n';
        total += std::stoi(line.substr(line.rfind(' ') + 1));
    }
    std::cout << "  total jobs handled: " << total << " (split between consumers varies per run)\n";
}
