// ============================================================
// Standard Algorithms
// ============================================================
// <algorithm> and <numeric> hold over 100 tested, optimised
// algorithms. A named algorithm says WHAT happens; a raw loop
// makes the reader work it out. "No raw loops" (Sean Parent) is a
// good default.
//
// This file uses the classic iterator-pair versions you will see
// in most existing code. 06_ranges_and_views.cpp shows the C++20
// std::ranges versions, which take the container directly.
//
// Build and run:
//   cmake --build build --target collections_05_algorithms
//   ./build/bin/collections_05_algorithms
// ============================================================

#include <algorithm>
#include <functional>   // std::greater, std::multiplies
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

void print(const std::string& label, const std::vector<int>& v)
{
    std::cout << "  " << label << ':';
    for (int x : v) std::cout << ' ' << x;
    std::cout << '\n';
}

// ------------------------------------------------------------
// Searching and Counting
// ------------------------------------------------------------

void searching()
{
    std::cout << "=== Searching and Counting ===\n";
    std::vector<int> v{4, 8, 15, 16, 23, 42};

    auto it = std::find(v.begin(), v.end(), 15);
    if (it != v.end()) std::cout << "  found 15 at index " << (it - v.begin()) << '\n';

    auto big = std::find_if(v.begin(), v.end(), [](int x) { return x > 20; });
    std::cout << "  first > 20: " << *big << '\n';

    auto evens = std::count_if(v.begin(), v.end(), [](int x) { return x % 2 == 0; });
    std::cout << "  even count: " << evens << '\n';

    std::cout << "  all positive? " << std::boolalpha
              << std::all_of(v.begin(), v.end(), [](int x) { return x > 0; }) << '\n';
    std::cout << "  any == 42?    " << std::any_of(v.begin(), v.end(), [](int x) { return x == 42; }) << '\n';

    // Binary search requires a SORTED range: O(log n).
    std::cout << "  binary_search(23): " << std::binary_search(v.begin(), v.end(), 23) << '\n';
    auto lb = std::lower_bound(v.begin(), v.end(), 20);   // first element >= 20
    std::cout << "  lower_bound(20) -> " << *lb << '\n';
}

// ------------------------------------------------------------
// Sorting and Ordering
// ------------------------------------------------------------
// std::sort: fast, not stable (equal elements may be reordered).
// std::stable_sort: keeps the original order of equal elements.
// std::partial_sort / nth_element: when you only need the top k.

struct Employee {
    std::string name;
    std::string team;
    int salary;
};

void sorting()
{
    std::cout << "\n=== Sorting ===\n";

    std::vector<int> v{5, 2, 9, 1, 7, 3};
    std::sort(v.begin(), v.end());
    print("sorted", v);
    std::sort(v.begin(), v.end(), std::greater<>{});
    print("descending", v);

    std::vector<int> scores{70, 95, 40, 88, 61, 99, 53};
    std::partial_sort(scores.begin(), scores.begin() + 3, scores.end(), std::greater<>{});
    std::cout << "  top 3: " << scores[0] << ' ' << scores[1] << ' ' << scores[2] << '\n';

    std::vector<Employee> staff{
        {"Ada", "eng", 70}, {"Bob", "ops", 50}, {"Cy", "eng", 50}, {"Di", "ops", 70},
    };
    // Sort by salary; stable_sort keeps the original order within equal salaries.
    std::stable_sort(staff.begin(), staff.end(),
                     [](const Employee& a, const Employee& b) { return a.salary < b.salary; });
    std::cout << "  by salary (stable):";
    for (const auto& e : staff) std::cout << ' ' << e.name << '(' << e.salary << ')';
    std::cout << '\n';
}

// ------------------------------------------------------------
// Transforming and Accumulating
// ------------------------------------------------------------
// std::transform  maps each element (like map() elsewhere)
// std::accumulate folds left to right (like reduce)
// std::reduce     like accumulate but may reorder: parallelisable

void transforming()
{
    std::cout << "\n=== Transforming and Accumulating ===\n";

    std::vector<int> v{1, 2, 3, 4};
    std::vector<int> squares(v.size());
    std::transform(v.begin(), v.end(), squares.begin(), [](int x) { return x * x; });
    print("squares", squares);

    int sum = std::accumulate(v.begin(), v.end(), 0);
    int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<>{});
    std::cout << "  sum = " << sum << ", product = " << product << '\n';

    // The type of the initial value decides the arithmetic: 0 sums
    // as int, 0.0 as double. A classic bug is summing doubles from 0.
    std::vector<double> prices{1.5, 2.25};
    std::cout << "  accumulate(..., 0.0) = " << std::accumulate(prices.begin(), prices.end(), 0.0)
              << " (with 0.0; using 0 would truncate each step to int)\n";

    std::vector<int> running(v.size());
    std::partial_sum(v.begin(), v.end(), running.begin());
    print("running total", running);

    std::vector<int> seq(5);
    std::iota(seq.begin(), seq.end(), 10);   // 10 11 12 13 14
    print("iota from 10", seq);
}

// ------------------------------------------------------------
// Removing, Deduplicating, Partitioning
// ------------------------------------------------------------
// std::remove does NOT shrink the container: it moves the kept
// elements to the front and returns the new logical end. Call
// erase afterwards (the erase-remove idiom), or use C++20
// std::erase / std::erase_if which do both.

void removing()
{
    std::cout << "\n=== Removing and Partitioning ===\n";

    std::vector<int> v{3, 1, 3, 2, 1, 3};
    auto new_end = std::remove(v.begin(), v.end(), 3);
    std::cout << "  after std::remove, size is still " << v.size() << '\n';
    v.erase(new_end, v.end());
    print("after erase", v);

    std::vector<int> dupes{1, 1, 2, 2, 2, 3, 1};
    std::sort(dupes.begin(), dupes.end());
    dupes.erase(std::unique(dupes.begin(), dupes.end()), dupes.end());   // unique needs sorted input
    print("deduplicated", dupes);

    std::vector<int> nums{1, 2, 3, 4, 5, 6, 7, 8};
    auto boundary = std::partition(nums.begin(), nums.end(), [](int x) { return x % 2 == 0; });
    std::cout << "  partitioned: " << (boundary - nums.begin()) << " evens first\n";

    auto [lo, hi] = std::minmax_element(nums.begin(), nums.end());
    std::cout << "  min " << *lo << ", max " << *hi << '\n';
}

int main()
{
    searching();
    sorting();
    transforming();
    removing();
}
