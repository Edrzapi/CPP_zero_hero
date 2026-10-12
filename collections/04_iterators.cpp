// ============================================================
// Iterators
// ============================================================
// Iterators are the glue between containers and algorithms: a
// generalised pointer with *, ++ and comparison. An algorithm is
// written once against iterators and then works on every
// container. Java's Iterator and C#'s IEnumerator are similar, but
// C++ uses a PAIR of iterators [begin, end) to describe a range.
//
// Build and run:
//   cmake --build build --target collections_04_iterators
//   ./build/bin/collections_04_iterators
// ============================================================

#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// ------------------------------------------------------------
// [begin, end): Half-Open Ranges
// ------------------------------------------------------------
// begin() points at the first element, end() ONE PAST the last.
// An empty range has begin() == end(). Never dereference end().

void half_open()
{
    std::cout << "=== begin and end ===\n";
    std::vector<int> v{10, 20, 30};

    for (auto it = v.begin(); it != v.end(); ++it) {
        *it += 1;                          // modify through the iterator
    }
    for (auto it = v.cbegin(); it != v.cend(); ++it) {   // const iterators: read only
        std::cout << "  " << *it;
    }
    std::cout << '\n';

    std::cout << "  reverse:";
    for (auto it = v.rbegin(); it != v.rend(); ++it) {
        std::cout << ' ' << *it;
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// Iterator Categories
// ------------------------------------------------------------
//   input / output   one pass (reading a stream)
//   forward          multi-pass, ++ only (forward_list, unordered_*)
//   bidirectional    ++ and -- (list, set, map)
//   random access    +n, -n, [], < (deque)
//   contiguous       random access + elements adjacent in memory (vector, array, string)
// Algorithms state what they need: std::sort needs random access,
// which is why std::list has its own member sort().

void categories()
{
    std::cout << "\n=== Categories and Iterator Arithmetic ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};
    auto mid = v.begin() + 2;                         // random access: jump directly
    std::cout << "  *(begin + 2) = " << *mid << ", distance = " << std::distance(v.begin(), mid) << '\n';

    std::list<int> l{1, 2, 3, 4, 5};
    auto lit = std::next(l.begin(), 2);               // bidirectional: walks 2 steps
    // auto bad = l.begin() + 2;                      // error: list iterators have no +
    std::cout << "  std::next(list.begin(), 2) -> " << *lit << '\n';

    std::set<int> s{3, 1, 2};
    // *s.begin() = 10;                               // error: set elements are const (they define the order)
    std::cout << "  smallest in set: " << *s.begin() << '\n';
}

// ------------------------------------------------------------
// Iterator Adaptors
// ------------------------------------------------------------
// back_inserter: an output iterator that calls push_back, so an
//   algorithm can grow a container it writes into.
// istream_iterator / ostream_iterator: treat streams as ranges.

void adaptors()
{
    std::cout << "\n=== Iterator Adaptors ===\n";

    std::vector<int> source{1, 2, 3, 4, 5, 6};
    std::vector<int> evens;
    std::copy_if(source.begin(), source.end(), std::back_inserter(evens),
                 [](int x) { return x % 2 == 0; });
    // Without back_inserter, copying into an EMPTY vector writes past its end: UB.

    std::cout << "  evens: ";
    std::copy(evens.begin(), evens.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << '\n';

    std::istringstream input{"7 3 9 1"};
    std::vector<int> parsed{std::istream_iterator<int>{input}, std::istream_iterator<int>{}};
    std::cout << "  parsed from a stream: " << parsed.size() << " numbers, max "
              << *std::max_element(parsed.begin(), parsed.end()) << '\n';
}

// ------------------------------------------------------------
// Invalidation Rules (Summary)
// ------------------------------------------------------------
//   vector/string  insert/push_back: all if reallocated, else after the point
//                  erase: at and after the point
//   deque          insert at ends: all iterators (references survive)
//   list/set/map   only the erased element's iterators
//   unordered_*    rehash invalidates all iterators (not references)
//
// Erasing while iterating: use the iterator that erase() returns.

void erase_while_iterating()
{
    std::cout << "\n=== Erase While Iterating ===\n";
    std::list<int> l{1, 2, 3, 4, 5, 6};
    for (auto it = l.begin(); it != l.end();) {
        if (*it % 2 == 0) {
            it = l.erase(it);          // erase returns the next valid iterator
        } else {
            ++it;
        }
    }
    std::cout << "  odds left:";
    for (int x : l) std::cout << ' ' << x;
    std::cout << '\n';
}

int main()
{
    half_open();
    categories();
    adaptors();
    erase_while_iterating();
}
