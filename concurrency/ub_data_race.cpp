// ============================================================
// A Data Race (Opt-In, Deliberate Undefined Behaviour)
// ============================================================
// NOT built by default. Build with:
//   cmake -S . -B build -DCPPZH_BUILD_UB_DEMOS=ON
//   cmake --build build --target concurrency_ub_data_race
//   ./build/bin/concurrency_ub_data_race
//
// The C++ version of Java Zero to Hero's RaceConditionRunner.
// Several threads do ++count on a plain int. In Java that merely
// loses updates. In C++ a data race is UNDEFINED BEHAVIOUR: the
// optimiser may assume no other thread touches "count", keep it in
// a register, and add 100000 in one go, so you might see the right
// answer, a wrong one, or (in theory) anything at all.
//
// Typical results: a Debug build loses updates; an optimised build
// may print exactly the expected value because each loop collapsed
// to a single addition. Neither means the code is correct.
//
// Detect it with ThreadSanitizer (GCC/Clang, Linux/macOS):
//   g++ -std=c++20 -g -O1 -fsanitize=thread ub_data_race.cpp -o race && ./race
// TSan prints "WARNING: ThreadSanitizer: data race" with both stacks.
// (TSan cannot be combined with AddressSanitizer in one build, so
// it is not part of the CPPZH_SANITIZE option.)
//
// The fixes are in 02_mutexes.cpp (std::mutex) and 04_atomics.cpp
// (std::atomic).
// ============================================================

#include <iostream>
#include <thread>
#include <vector>

constexpr int kThreads = 8;
constexpr int kIncrements = 100'000;

int g_count = 0;   // shared, unprotected: the bug

int main()
{
    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([] {
            for (int i = 0; i < kIncrements; ++i) {
                ++g_count;   // read, add, write: three steps that can interleave
            }
        });
    }
    for (auto& th : threads) th.join();

    std::cout << "=== Data Race ===\n";
    std::cout << "  expected " << kThreads * kIncrements << ", got " << g_count << '\n';
    std::cout << "  run it several times, and try a Release build\n";
}
