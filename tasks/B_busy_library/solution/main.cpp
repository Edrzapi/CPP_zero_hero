// ============================================================
// Task B: Busy Library (solution, Core + Stretch)
// ============================================================
// Bonus session: concurrency (see concurrency/). Many members
// borrow at once.
//
// The race: borrowing is "check, then act":
//     if (available > 0) {   // 1. check
//         --available;       // 2. act
//     }
// Two threads can both pass step 1 before either does step 2,
// and the library lends a copy it does not have. Even
// --available on its own is read, subtract, write: two threads
// can read the same value and one update is lost.
//
// The fix: one mutex around the whole check-and-act, so only one
// thread at a time can be inside it.
//
// Build and run:
//   cmake --build build --target taskB_solution
//   ./build/bin/taskB_solution             Core: the safe version (checked)
//   ./build/bin/taskB_solution --stretch   Core, then the race without the lock
// Check (from the repository root; run it several times):
//   py -3.13 tasks/check/check.py B -- build/bin/taskB_solution
// ============================================================

#include <atomic>
#include <future>
#include <iostream>
#include <latch>
#include <mutex>
#include <string_view>
#include <thread>
#include <vector>

constexpr int MEMBERS_1 = 8;
constexpr int COPIES_1 = 3;
constexpr int MEMBERS_2 = 4;
constexpr int COPIES_2 = 4;
constexpr int CYCLES = 5000;

// ------------------------------------------------------------
// Shelf: the Copies of One Book, Shared by Every Thread
// ------------------------------------------------------------

class Shelf {
public:
    explicit Shelf(int copies) : copies_{copies}, available_{copies} {}

    // The fix: check and act together, under one lock.
    // std::lock_guard locks in its constructor and unlocks in its
    // destructor, so the mutex is released on every return path.
    bool borrow()
    {
        std::lock_guard<std::mutex> lock{mutex_};   // one thread at a time from here...
        if (available_ > 0) {
            --available_;
            ++loans_recorded_;
            return true;
        }
        return false;
    }   // ...to here

    void give_back()
    {
        std::lock_guard<std::mutex> lock{mutex_};
        ++available_;
    }

    int copies() const { return copies_; }

    int available()
    {
        std::lock_guard<std::mutex> lock{mutex_};   // reading shared data needs the lock too
        return available_;
    }

    int loans_recorded()
    {
        std::lock_guard<std::mutex> lock{mutex_};
        return loans_recorded_;
    }

private:
    const int copies_;
    int available_;
    int loans_recorded_ = 0;
    std::mutex mutex_;
};

// ------------------------------------------------------------
// Stretch: the Same Shelf with No Lock
// ------------------------------------------------------------
// With plain ints this would be a data race, which is undefined
// behaviour in C++: the program may do anything, not just give
// wrong numbers (see concurrency/ub_data_race.cpp, opt-in only).
// Atomics make each single read and write safe, but the check
// and the act are still separate steps, so the race condition
// (the logic bug) is still there. That is the lesson: atomic
// pieces do not make an atomic whole. yield() gives other
// threads a chance to run in the gap, as a busy machine would.

class UnsafeShelf {
public:
    explicit UnsafeShelf(int copies) : available_{copies} {}

    bool borrow()
    {
        if (available_.load() > 0) {          // 1. check
            std::this_thread::yield();
            available_.store(available_.load() - 1);   // 2. act: read, subtract, write
            const int count = loans_recorded_.load();
            std::this_thread::yield();
            loans_recorded_.store(count + 1);
            return true;
        }
        return false;
    }

    void give_back()
    {
        const int value = available_.load();
        std::this_thread::yield();
        available_.store(value + 1);
    }

    int available() const { return available_.load(); }
    int loans_recorded() const { return loans_recorded_.load(); }

private:
    std::atomic<int> available_;
    std::atomic<int> loans_recorded_{0};
};

// ------------------------------------------------------------
// The Two Scenarios (templates, so both shelves can use them)
// ------------------------------------------------------------

struct Scenario1 {
    int borrowed;
    int refused;
    int available;
};

// MEMBERS_1 threads each try once to borrow from COPIES_1 copies.
// A std::latch (C++20) holds every thread until all of them have
// arrived, so they really do try at the same moment.
template <typename ShelfType>
Scenario1 scenario_1()
{
    ShelfType shelf{COPIES_1};
    std::latch start{MEMBERS_1};
    std::atomic<int> borrowed{0};
    std::atomic<int> refused{0};

    std::vector<std::thread> members;
    for (int i = 0; i < MEMBERS_1; ++i) {
        members.emplace_back([&] {
            start.arrive_and_wait();   // count down, then wait for the others
            if (shelf.borrow()) {
                ++borrowed;
            } else {
                ++refused;
            }
        });
    }
    for (std::thread& member : members) {
        member.join();   // wait for every thread before reading the results
    }
    return {borrowed.load(), refused.load(), shelf.available()};
}

struct Scenario2 {
    int loans;
    int available;
};

// MEMBERS_2 members each borrow and give back CYCLES times.
// Stretch 2: std::async is C++'s task API. Each call runs the
// lambda on its own thread (std::launch::async) and returns a
// future; get() waits for it to finish and passes on any
// exception the task threw.
template <typename ShelfType>
Scenario2 scenario_2()
{
    ShelfType shelf{COPIES_2};
    std::vector<std::future<void>> members;
    for (int i = 0; i < MEMBERS_2; ++i) {
        members.push_back(std::async(std::launch::async, [&shelf] {
            for (int cycle = 0; cycle < CYCLES; ++cycle) {
                if (shelf.borrow()) {
                    shelf.give_back();
                }
            }
        }));
    }
    for (std::future<void>& member : members) {
        member.get();
    }
    return {shelf.loans_recorded(), shelf.available()};
}

int main(int argc, char* argv[])
{
    std::cout << "Busy library\n";
    std::cout << "Scenario 1: " << MEMBERS_1 << " members, 1 book with " << COPIES_1
              << " copies, everyone borrows at once\n";
    const Scenario1 first = scenario_1<Shelf>();
    std::cout << "borrowed: " << first.borrowed << '\n';
    std::cout << "refused: " << first.refused << '\n';
    std::cout << "available at end: " << first.available << '\n';

    std::cout << "Scenario 2: " << MEMBERS_2 << " members, " << COPIES_2 << " copies, " << CYCLES
              << " borrow-and-return cycles each\n";
    const Scenario2 second = scenario_2<Shelf>();
    std::cout << "loans recorded: " << second.loans << '\n';
    std::cout << "available at end: " << second.available << '\n';

    const bool ok = first.borrowed == COPIES_1 && first.refused == MEMBERS_1 - COPIES_1 && first.available == 0 &&
                    second.loans == MEMBERS_2 * CYCLES && second.available == COPIES_2;
    std::cout << "Result: " << (ok ? "OK" : "WRONG") << '\n';

    if (argc > 1 && std::string_view{argv[1]} == "--stretch") {
        // Not checked: the numbers change from run to run, which is the point
        std::cout << "Stretch: the same scenarios with no lock\n";
        const Scenario1 racy_first = scenario_1<UnsafeShelf>();
        std::cout << "scenario 1: borrowed " << racy_first.borrowed << " of " << COPIES_1
                  << " copies, available at end " << racy_first.available << '\n';
        const Scenario2 racy_second = scenario_2<UnsafeShelf>();
        std::cout << "scenario 2: loans recorded " << racy_second.loans << " of " << MEMBERS_2 * CYCLES
                  << ", available at end " << racy_second.available << '\n';
    }
}
