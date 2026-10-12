// ============================================================
// std::span: A View over Contiguous Elements (C++20)
// ============================================================
// The C idiom "pointer + length" made safe and convenient. A
// span<T> refers to elements owned by someone else (C array,
// std::array, std::vector) and carries the size with it. Like
// std::string_view for text, it is cheap to copy and pass by
// value, and it must not outlive the data it points to.
//
// Build and run:
//   cmake --build build --target collections_07_span
//   ./build/bin/collections_07_span
// ============================================================

#include <array>
#include <iostream>
#include <numeric>
#include <span>
#include <vector>

// ------------------------------------------------------------
// One Function, Any Contiguous Source
// ------------------------------------------------------------
// Before span you needed overloads or templates, or the C style
// (const int* data, std::size_t size) and hoped the size matched.

double average(std::span<const int> values)   // read-only view: span<const T>
{
    if (values.empty()) return 0.0;
    return static_cast<double>(std::accumulate(values.begin(), values.end(), 0)) /
           static_cast<double>(values.size());
}

void double_all(std::span<int> values)        // writable view: changes the caller's data
{
    for (int& v : values) v *= 2;
}

void any_source()
{
    std::cout << "=== One Function, Many Containers ===\n";

    int c_array[] = {1, 2, 3, 4};
    std::array<int, 3> fixed{10, 20, 30};
    std::vector<int> dynamic{5, 15};

    std::cout << "  average(C array)   = " << average(c_array) << '\n';
    std::cout << "  average(std::array)= " << average(fixed) << '\n';
    std::cout << "  average(vector)    = " << average(dynamic) << '\n';

    double_all(dynamic);
    std::cout << "  after double_all: " << dynamic[0] << ", " << dynamic[1] << '\n';
}

// ------------------------------------------------------------
// Sub-Spans
// ------------------------------------------------------------
// first(n), last(n), subspan(offset, count): slices with no copy.

void slicing()
{
    std::cout << "\n=== Slicing Without Copying ===\n";
    std::vector<int> readings{3, 9, 4, 8, 6, 7, 2};
    std::span<const int> all{readings};

    std::cout << "  first 3 avg: " << average(all.first(3)) << '\n';
    std::cout << "  last 2 avg:  " << average(all.last(2)) << '\n';
    std::cout << "  middle 3:   ";
    for (int x : all.subspan(2, 3)) std::cout << ' ' << x;
    std::cout << '\n';
}

// ------------------------------------------------------------
// Fixed vs Dynamic Extent
// ------------------------------------------------------------
// span<int, 4> knows its size at compile time and is just a
// pointer. span<int> (dynamic extent) stores pointer + size.

void extents()
{
    std::cout << "\n=== Static and Dynamic Extent ===\n";
    std::array<int, 4> quad{1, 2, 3, 4};
    std::span<int, 4> fixed{quad};
    std::span<int> dynamic{quad};
    std::cout << "  sizeof(span<int, 4>) = " << sizeof(fixed)
              << ", sizeof(span<int>) = " << sizeof(dynamic) << '\n';
}

// ------------------------------------------------------------
// Lifetime: the Same Rule as string_view
// ------------------------------------------------------------
//   std::span<int> s = std::vector<int>{1, 2, 3};   // dangles at the semicolon
//   std::vector<int> v{1, 2, 3};
//   std::span<int> s2{v};
//   v.push_back(4);                                 // may reallocate: s2 dangles
// A span is a borrowed view; the owner must stay alive and must
// not reallocate while the span is in use.

int main()
{
    any_source();
    slicing();
    extents();
}
