// ============================================================
// Range-Based for: Under the Hood
// ============================================================
// What the compiler turns a range-based for loop into, how to
// make your own class iterable, and the temporary-lifetime trap
// that catches experienced developers.
//
// Build and run:
//   cmake --build build --target iteration_02_range_for_under_the_hood
//   ./build/bin/iteration_02_range_for_under_the_hood
// ============================================================

#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// ------------------------------------------------------------
// The Desugaring
// ------------------------------------------------------------
//   for (const auto& x : range) body;
//
// becomes, roughly:
//
//   {
//       auto&& __range = range;
//       auto __begin = __range.begin();   // or begin(__range)
//       auto __end   = __range.end();
//       for (; __begin != __end; ++__begin) {
//           const auto& x = *__begin;
//           body;
//       }
//   }
//
// So any type works if it has begin()/end() returning something
// that supports !=, ++ and *. That is the whole "protocol".

void desugared()
{
    std::cout << "=== The Loop Written Out by Hand ===\n";
    std::vector<int> values{1, 2, 3};

    for (auto it = values.begin(), end = values.end(); it != end; ++it) {
        const auto& x = *it;
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// Making Your Own Class Iterable
// ------------------------------------------------------------
// A Range of integers [first, last) with a step, like Python's
// range(). The iterator is a tiny class with *, ++ and ==.
// (In real code, std::views::iota already does this.)

class Range {
public:
    class Iterator {
    public:
        // These aliases let standard algorithms reason about the iterator.
        using iterator_category = std::forward_iterator_tag;
        using value_type = int;
        using difference_type = std::ptrdiff_t;
        using pointer = const int*;
        using reference = int;

        Iterator(int current, int step) : current_{current}, step_{step} {}

        int operator*() const { return current_; }

        Iterator& operator++()
        {
            current_ += step_;
            return *this;
        }

        // C++20: defining == also gives you != for free.
        bool operator==(const Iterator& other) const
        {
            // Stop when we reach OR pass the end, so steps that skip over it still finish.
            return step_ > 0 ? current_ >= other.current_ : current_ <= other.current_;
        }

    private:
        int current_;
        int step_;
    };

    Range(int first, int last, int step = 1) : first_{first}, last_{last}, step_{step} {}

    Iterator begin() const { return {first_, step_}; }
    Iterator end() const { return {last_, step_}; }

private:
    int first_;
    int last_;
    int step_;
};

void custom_range()
{
    std::cout << "\n=== A Custom Iterable ===\n";

    for (int i : Range(0, 10, 3)) {
        std::cout << i << ' ';          // 0 3 6 9
    }
    std::cout << '\n';

    for (int i : Range(5, 0, -1)) {
        std::cout << i << ' ';          // 5 4 3 2 1
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// The Temporary Lifetime Trap
// ------------------------------------------------------------
// auto&& __range = range;  extends the life of a temporary
// returned by the range expression, but NOT of temporaries
// INSIDE it. Before C++23:
//
//   for (char c : make_name().substr(0, 3))   // fine: substr's result is kept alive
//   for (int x : get_holder().items())        // DANGLING if items() returns a reference
//                                             // into the temporary holder
//
// C++23 (P2718) extends all temporaries in the range expression.
// Portable fix: give the temporary a name first.

struct Holder {
    std::vector<int> data{7, 8, 9};
    const std::vector<int>& items() const { return data; }
};

Holder make_holder() { return Holder{}; }

void lifetime_trap()
{
    std::cout << "\n=== Naming the Temporary ===\n";

    // for (int x : make_holder().items())   // UB before C++23: the Holder dies first
    const Holder holder = make_holder();     // safe in every standard
    for (int x : holder.items()) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// Iterator Invalidation While Looping
// ------------------------------------------------------------
// Adding to or erasing from a vector inside a range-for over the
// same vector can invalidate the hidden iterators: undefined
// behaviour. Erase with std::erase_if (C++20) instead.

void erase_while_iterating()
{
    std::cout << "\n=== Removing Elements Safely ===\n";

    std::vector<int> values{1, 2, 3, 4, 5, 6};
    // for (int v : values) if (v % 2 == 0) values.erase(...);   // never do this

    auto removed = std::erase_if(values, [](int v) { return v % 2 == 0; });
    std::cout << "removed " << removed << ", left: ";
    for (int v : values) std::cout << v << ' ';
    std::cout << '\n';
}

int main()
{
    desugared();
    custom_range();
    lifetime_trap();
    erase_while_iterating();
}
