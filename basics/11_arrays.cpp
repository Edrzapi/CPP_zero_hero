// ============================================================
// Arrays: C Arrays and std::array
// ============================================================
// C arrays have a fixed size, no bounds checking, cannot be
// copied with = and "decay" to a pointer at the first chance.
// std::array wraps one with the same performance and none of
// the surprises.
//
// Build and run:
//   cmake --build build --target basics_11_arrays
//   ./build/bin/basics_11_arrays
// ============================================================

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <iterator>   // std::size

// ------------------------------------------------------------
// C Arrays and Decay
// ------------------------------------------------------------
// Passing an array to a function passes a POINTER. Inside the
// function sizeof gives the pointer size, not the array size.
// That is why C APIs always take a length parameter.

void print_c_array(const int* data, std::size_t length)
{
    for (std::size_t i = 0; i < length; ++i) {
        std::cout << data[i] << ' ';
    }
    std::cout << '\n';
}

// "const int values[]" in a parameter list means exactly
// "const int* values". Written with [] and sizeof, GCC and Clang
// even warn (-Wsizeof-array-argument), so the pointer form is
// used here to show what the function really receives.
std::size_t size_inside_function(const int* values)
{
    return sizeof(values);   // the size of a pointer, NOT the array
}

// A reference to an array keeps the size as part of the type.
template <std::size_t N>
std::size_t size_by_reference(const int (&)[N])
{
    return N;
}

void c_arrays()
{
    std::cout << "=== C Arrays ===\n";

    int scores[5] = {90, 85, 70};    // remaining elements are zero
    std::cout << "sizeof(scores)                 = " << sizeof(scores) << '\n';      // 20
    std::cout << "element count (C idiom)        = " << sizeof(scores) / sizeof(scores[0]) << '\n';
    std::cout << "std::size(scores) (C++17)      = " << std::size(scores) << '\n';
    std::cout << "sizeof inside a function       = " << size_inside_function(scores) << '\n'; // 8 on 64-bit
    std::cout << "size via reference-to-array    = " << size_by_reference(scores) << '\n';

    print_c_array(scores, std::size(scores));

    // int copy[5] = scores;   // error: arrays cannot be copied with =
    // scores[5] = 1;          // compiles, but out of bounds: undefined behaviour
}

// ------------------------------------------------------------
// Multi-Dimensional Arrays
// ------------------------------------------------------------
// Stored row by row (row-major) in one contiguous block.

void two_dimensional()
{
    std::cout << "\n=== 2D Arrays ===\n";

    int grid[2][3] = {
        {1, 2, 3},
        {4, 5, 6},
    };

    for (const auto& row : grid) {
        for (int cell : row) {
            std::cout << cell << ' ';
        }
        std::cout << '\n';
    }
}

// ------------------------------------------------------------
// std::array
// ------------------------------------------------------------
// Same layout and speed as a C array, but it knows its size,
// can be copied, compared and returned, and has .at() for
// bounds-checked access.

std::array<int, 3> make_triple()
{
    return {1, 2, 3};   // returning an array by value: impossible with a C array
}

void std_array()
{
    std::cout << "\n=== std::array ===\n";

    std::array<int, 5> values{5, 3, 9, 1, 7};
    std::array<int, 5> copy = values;           // copies every element

    std::sort(values.begin(), values.end());
    std::cout << "sorted : ";
    for (int v : values) std::cout << v << ' ';
    std::cout << "\nsize() = " << values.size()
              << ", copy unchanged? " << std::boolalpha << (copy[0] == 5) << '\n';

    std::cout << "copy == values? " << (copy == values) << '\n';   // false, element-wise compare

    auto triple = make_triple();
    std::cout << "triple.back() = " << triple.back() << '\n';

    // CTAD (C++17) deduces the type and size:
    std::array deduced{1.5, 2.5};   // std::array<double, 2>
    std::cout << "deduced size  = " << deduced.size() << '\n';

    // .data() gives the underlying C array for C APIs.
    print_c_array(values.data(), values.size());
}

int main()
{
    c_arrays();
    two_dimensional();
    std_array();
}
