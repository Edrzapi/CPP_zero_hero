// ============================================================
// Ranges and Views (C++20)
// ============================================================
// std::ranges algorithms take a whole container instead of a
// begin/end pair, support projections, and are checked with
// concepts (much clearer errors). Views are LAZY, non-owning
// adaptors composed with | , like Java streams, C# LINQ or Python
// generator expressions.
//
// Build and run:
//   cmake --build build --target collections_06_ranges_and_views
//   ./build/bin/collections_06_ranges_and_views
// ============================================================

#include <algorithm>
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Range Algorithms
// ------------------------------------------------------------
// Same names as before, in namespace std::ranges.
// A PROJECTION transforms each element before comparing, which
// replaces many hand-written comparison lambdas.

struct Book {
    std::string title;
    int year;
};

void range_algorithms()
{
    std::cout << "=== std::ranges Algorithms ===\n";

    std::vector<int> v{5, 2, 9, 1};
    std::ranges::sort(v);                          // no begin()/end()
    std::cout << "  sorted:";
    for (int x : v) std::cout << ' ' << x;
    std::cout << '\n';

    std::vector<Book> books{{"Dune", 1965}, {"Neuromancer", 1984}, {"Foundation", 1951}};
    std::ranges::sort(books, {}, &Book::year);     // {} = default less, projection = year
    std::cout << "  by year:";
    for (const auto& b : books) std::cout << ' ' << b.title << '(' << b.year << ')';
    std::cout << '\n';

    auto it = std::ranges::find(books, std::string{"Dune"}, &Book::title);   // find by projected member
    if (it != books.end()) std::cout << "  found Dune from " << it->year << '\n';

    auto [min_it, max_it] = std::ranges::minmax_element(books, {}, &Book::year);
    std::cout << "  oldest " << min_it->title << ", newest " << max_it->title << '\n';
}

// ------------------------------------------------------------
// Views and Pipelines
// ------------------------------------------------------------
// A view describes a computation; nothing runs until you iterate.
// filter, transform, take, drop, reverse, iota, keys, values...
// Views do not own the elements: the underlying container must
// outlive the view.

void views()
{
    std::cout << "\n=== Views and Pipelines ===\n";

    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto even_squares = numbers
                        | std::views::filter([](int x) { return x % 2 == 0; })
                        | std::views::transform([](int x) { return x * x; });
    std::cout << "  even squares:";
    for (int x : even_squares) std::cout << ' ' << x;      // computed here, lazily
    std::cout << '\n';

    std::cout << "  iota(1) | take(5):";
    for (int x : std::views::iota(1) | std::views::take(5)) std::cout << ' ' << x;   // infinite source, safe
    std::cout << '\n';

    std::cout << "  reverse | drop(7):";
    for (int x : numbers | std::views::reverse | std::views::drop(7)) std::cout << ' ' << x;
    std::cout << '\n';

    std::map<std::string, int> ages{{"Ada", 36}, {"Alan", 41}};
    std::cout << "  keys:";
    for (const auto& name : ages | std::views::keys) std::cout << ' ' << name;
    std::cout << " | values:";
    for (int age : std::views::values(ages)) std::cout << ' ' << age;
    std::cout << '\n';

    // Laziness made visible: transform runs only for elements that are pulled.
    int calls = 0;
    auto counted = numbers | std::views::transform([&calls](int x) { ++calls; return x; })
                           | std::views::take(3);
    for (int x : counted) { (void)x; }
    std::cout << "  transform ran " << calls << " times for take(3) of 10 elements\n";
}

// ------------------------------------------------------------
// Materialising a View
// ------------------------------------------------------------
// C++23 adds std::ranges::to<std::vector>(). In C++20, build the
// container from the view's iterators. Views from filter are not
// "common ranges" (begin and end types differ), so views::common
// makes them usable with the iterator-pair constructor.

void materialise()
{
    std::cout << "\n=== Collecting Results (C++20) ===\n";

    std::vector<std::string> words{"apple", "fig", "banana", "kiwi"};
    auto long_words = words | std::views::filter([](const std::string& w) { return w.size() > 3; })
                            | std::views::common;
    std::vector<std::string> result(long_words.begin(), long_words.end());
    // C++23: auto result = words | std::views::filter(...) | std::ranges::to<std::vector>();

    std::cout << "  " << result.size() << " long words:";
    for (const auto& w : result) std::cout << ' ' << w;
    std::cout << '\n';
}

// ------------------------------------------------------------
// Dangling Protection
// ------------------------------------------------------------
// Range algorithms called on a TEMPORARY container return
// std::ranges::dangling instead of an iterator into memory that is
// about to be freed, turning a run-time bug into a compile error:
//
//   auto it = std::ranges::find(std::vector{1, 2, 3}, 2);
//   std::cout << *it;   // error: no operator* for std::ranges::dangling

int main()
{
    range_algorithms();
    views();
    materialise();
}
