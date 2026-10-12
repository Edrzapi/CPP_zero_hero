// ============================================================
// Threads: std::thread and std::jthread
// ============================================================
// std::thread (C++11) starts a function on a new OS thread.
// std::jthread (C++20) is the version you should normally use:
// it joins automatically in its destructor and supports
// cooperative cancellation through a stop_token.
//
// Build and run:
//   cmake --build build --target concurrency_01_threads
//   ./build/bin/concurrency_01_threads
// ============================================================

#include <chrono>
#include <exception>
#include <functional>   // std::ref
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <version>      // __cpp_lib_jthread

using namespace std::chrono_literals;

// std::cout is thread-safe in the sense that it will not crash,
// but output from several threads can interleave mid-line. A lock
// around each whole line keeps the demo output readable (more on
// mutexes in 02_mutexes.cpp).
std::mutex g_print_mutex;

void say(const std::string& text)
{
    std::lock_guard lock{g_print_mutex};
    std::cout << text << '\n';
}

// ------------------------------------------------------------
// Starting and Joining
// ------------------------------------------------------------
// A thread starts running as soon as the std::thread is created.
// Before a std::thread is destroyed you MUST call join() (wait for
// it) or detach() (let it run on its own). Destroying a joinable
// std::thread calls std::terminate.
// Any callable works: a function, a lambda, a functor, a member
// function with an object.

void worker(int id, int iterations)
{
    for (int i = 0; i < iterations; ++i) {
        std::this_thread::sleep_for(1ms);
    }
    say("  worker " + std::to_string(id) + " finished " + std::to_string(iterations) + " iterations");
}

void starting_threads()
{
    std::cout << "=== std::thread ===\n";
    std::cout << "  hardware threads available: " << std::thread::hardware_concurrency() << '\n';

    std::thread a{worker, 1, 5};                    // function + arguments (copied into the thread)
    std::thread b{[] { say("  lambda thread running"); }};

    a.join();
    b.join();

    // Several threads in a container.
    std::vector<std::thread> pool;
    for (int id = 2; id <= 4; ++id) {
        pool.emplace_back(worker, id, id);
    }
    for (auto& t : pool) {
        t.join();
    }
}

// ------------------------------------------------------------
// Passing Arguments
// ------------------------------------------------------------
// Arguments are COPIED into the new thread's storage, even if the
// function takes a reference. To really pass a reference, wrap it
// in std::ref, and make sure the object outlives the thread.

void add_to_total(int& total, int amount) { total += amount; }

void passing_arguments()
{
    std::cout << "\n=== Passing Arguments ===\n";
    int total = 0;
    std::thread t{add_to_total, std::ref(total), 10};   // without std::ref this does not compile
    t.join();                                           // join BEFORE reading total
    std::cout << "  total after thread: " << total << '\n';
}

// ------------------------------------------------------------
// Exceptions in Threads
// ------------------------------------------------------------
// An exception escaping a thread function calls std::terminate.
// Catch inside the thread and hand it over with std::exception_ptr
// (std::async in 05_async_and_future.cpp does this for you).

void exceptions_in_threads()
{
    std::cout << "\n=== Exceptions Across Threads ===\n";
    std::exception_ptr error;
    std::thread t{[&error] {
        try {
            throw std::runtime_error{"failure inside the thread"};
        } catch (...) {
            error = std::current_exception();
        }
    }};
    t.join();
    try {
        if (error) std::rethrow_exception(error);
    } catch (const std::exception& e) {
        std::cout << "  main thread caught: " << e.what() << '\n';
    }
}

// ------------------------------------------------------------
// std::jthread and stop_token (C++20)
// ------------------------------------------------------------
// The destructor requests stop and joins: no terminate, no leak.
// If the function's first parameter is a std::stop_token, the
// jthread passes one in, and the thread can poll it to finish
// cleanly when asked.

void jthread_demo()
{
    std::cout << "\n=== std::jthread (C++20) ===\n";
#if defined(__cpp_lib_jthread)
    std::jthread ticker{[](std::stop_token stop) {
        int ticks = 0;
        while (!stop.stop_requested()) {
            ++ticks;
            std::this_thread::sleep_for(5ms);
        }
        say("  ticker stopped cleanly after " + std::to_string(ticks) + " ticks");
    }};

    std::this_thread::sleep_for(30ms);
    ticker.request_stop();   // optional: the destructor would do it too
    // ticker.join() is also optional: the destructor joins.
#else
    std::cout << "  std::jthread is not available in this standard library yet.\n";
#endif
}

// ------------------------------------------------------------
// detach: Usually a Mistake
// ------------------------------------------------------------
// A detached thread keeps running after its std::thread object is
// gone, with no way to wait for it. If main returns while it is
// still running, it is killed mid-work, and anything it references
// may already be destroyed. Prefer jthread or a thread pool.

int main()
{
    starting_threads();
    passing_arguments();
    exceptions_in_threads();
    jthread_demo();
}
