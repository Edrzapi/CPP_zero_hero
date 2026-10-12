// ============================================================
// Task B: Busy Library (starter)
// ============================================================
// Read ../README.md first. This task stands alone: one Shelf, no
// files. Shelf is an outline and the scenarios are stubs.
//
// Build and run:
//   cmake --build build --target taskB_starter
//   ./build/bin/taskB_starter
// Check (from the repository root; run it several times):
//   py -3.13 tasks/check/check.py B -- build/bin/taskB_starter
// ============================================================

#include <iostream>
#include <mutex>

constexpr int MEMBERS_1 = 8;
constexpr int COPIES_1 = 3;
constexpr int MEMBERS_2 = 4;
constexpr int COPIES_2 = 4;
constexpr int CYCLES = 5000;

// TODO Core 1: borrow() and give_back() each lock mutex_ around
// the whole check-and-act (std::lock_guard<std::mutex>).
class Shelf {
public:
    explicit Shelf(int copies) : copies_{copies}, available_{copies} {}

    bool borrow() { return false; }   // not implemented
    void give_back() {}               // not implemented

    int copies() const { return copies_; }
    int available() const { return available_; }
    int loans_recorded() const { return loans_recorded_; }

private:
    const int copies_;
    int available_;
    int loans_recorded_ = 0;
    std::mutex mutex_;
};

int main()
{
    std::cout << "Busy library\n";
    std::cout << "Scenario 1: " << MEMBERS_1 << " members, 1 book with " << COPIES_1
              << " copies, everyone borrows at once\n";
    // TODO Core 2: MEMBERS_1 std::threads wait on a std::latch, then
    // each borrows once. join() them all, then print borrowed,
    // refused and available at end.
    std::cout << "not implemented: scenario 1\n";

    std::cout << "Scenario 2: " << MEMBERS_2 << " members, " << COPIES_2 << " copies, " << CYCLES
              << " borrow-and-return cycles each\n";
    // TODO Core 3: MEMBERS_2 threads, each CYCLES times: borrow, then
    // give back. Print loans recorded and available at end.
    std::cout << "not implemented: scenario 2\n";

    // TODO Core 4: "Result: OK" or "Result: WRONG"
    std::cout << "not implemented: result\n";
}
