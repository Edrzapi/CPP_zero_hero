// ============================================================
// std::async, Futures and Promises
// ============================================================
// Task-based concurrency: ask for a RESULT computed elsewhere and
// collect it later, instead of managing threads by hand. A
// std::future<T> is a one-shot channel that will eventually hold a
// T or an exception. Like Java's Future / CompletableFuture and
// C#'s Task<T> (without async/await syntax: C++20 coroutines exist
// but the standard library offers no task type for them yet).
//
// Build and run:
//   cmake --build build --target concurrency_05_async_and_future
//   ./build/bin/concurrency_05_async_and_future
// ============================================================

#include <chrono>
#include <cstddef>
#include <functional>   // std::cref
#include <future>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ------------------------------------------------------------
// std::async
// ------------------------------------------------------------
// Launch policies:
//   std::launch::async     run on a new thread now
//   std::launch::deferred  run lazily, on the calling thread, when get() is called
//   default (both)         the implementation chooses: specify async
//                          if you actually want parallelism
//
// get() waits for the result and can be called ONCE.
// Gotcha: the future returned by std::async blocks in its
// destructor until the task finishes, so discarding it makes the
// call synchronous.

long long sum_range(const std::vector<int>& data, std::size_t begin, std::size_t end)
{
    return std::accumulate(data.begin() + static_cast<std::ptrdiff_t>(begin),
                           data.begin() + static_cast<std::ptrdiff_t>(end), 0LL);
}

void async_demo()
{
    std::cout << "=== std::async: a Parallel Sum ===\n";

    std::vector<int> data(1'000'000);
    std::iota(data.begin(), data.end(), 1);

    const std::size_t half = data.size() / 2;
    auto first_half = std::async(std::launch::async, sum_range, std::cref(data), std::size_t{0}, half);
    long long second_half = sum_range(data, half, data.size());   // this thread does the other half

    long long total = first_half.get() + second_half;
    std::cout << "  sum 1..1000000 = " << total << " (expected 500000500000)\n";

    auto lazy = std::async(std::launch::deferred, [] { return 42; });
    std::cout << "  deferred task ran only when asked: " << lazy.get() << '\n';
}

// ------------------------------------------------------------
// Exceptions Travel Through the Future
// ------------------------------------------------------------
// An exception thrown by the task is stored in the future and
// rethrown by get() on the waiting thread.

void exception_demo()
{
    std::cout << "\n=== Exceptions Through a Future ===\n";
    auto failing = std::async(std::launch::async, []() -> int {
        throw std::runtime_error{"task failed"};
    });
    try {
        std::cout << failing.get();
    } catch (const std::exception& e) {
        std::cout << "  get() rethrew: " << e.what() << '\n';
    }
}

// ------------------------------------------------------------
// std::promise: Setting a Future's Value by Hand
// ------------------------------------------------------------
// The promise is the WRITING end, the future the READING end.
// Useful when the result is produced by code that is not a simple
// function call, such as a callback or a long-running thread.

void promise_demo()
{
    std::cout << "\n=== promise / future ===\n";
    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();

    std::thread producer{[p = std::move(promise)]() mutable {
        std::this_thread::sleep_for(10ms);
        p.set_value("result from the producer thread");
    }};

    // wait_for lets the consumer do other work or time out.
    while (future.wait_for(1ms) != std::future_status::ready) {
        // could do something useful here
    }
    std::cout << "  " << future.get() << '\n';
    producer.join();
}

// ------------------------------------------------------------
// std::packaged_task and std::shared_future
// ------------------------------------------------------------
// packaged_task wraps a callable so that calling it fills a
// future: handy for building job queues and thread pools.
// shared_future allows several threads to get() the same result.

void packaged_task_demo()
{
    std::cout << "\n=== packaged_task and shared_future ===\n";

    std::packaged_task<int(int, int)> task{[](int a, int b) { return a * b; }};
    std::shared_future<int> result = task.get_future().share();

    std::thread runner{std::move(task), 6, 7};

    std::vector<std::thread> readers;
    std::vector<int> seen(3);
    for (std::size_t i = 0; i < seen.size(); ++i) {
        readers.emplace_back([result, &seen, i] { seen[i] = result.get(); });   // each copy may get()
    }
    runner.join();
    for (auto& r : readers) r.join();
    std::cout << "  three readers saw: " << seen[0] << ' ' << seen[1] << ' ' << seen[2] << '\n';
}

int main()
{
    async_demo();
    exception_demo();
    promise_demo();
    packaged_task_demo();
}
