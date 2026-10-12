// ============================================================
// C++ Zero to Hero: Entry Point
// ============================================================
// Every example is its own executable, named <folder>_<file>.
// Build and run one:
//   cmake --build build --target basics_02_types_and_sizes
//   ./build/bin/basics_02_types_and_sizes
// Run a whole topic in order:
//   cmake --build build --target run_basics
// Run everything as a smoke test:
//   ctest --test-dir build --output-on-failure
// ============================================================

#include <iostream>
#include <string_view>

struct Topic {
    std::string_view folder;
    std::string_view summary;
};

constexpr Topic kSyllabus[] = {
    {"basics", "types, sizes, signed/unsigned, literals, const, references, initialisation"},
    {"control_flow", "if/else, if with initialiser, switch, if constexpr"},
    {"iteration", "loops, range-for under the hood, generators (C++23)"},
    {"functions", "parameters, overloading, defaults, inline, lambdas, function pointers"},
    {"compilation", "preprocessor, translation units, headers, linkage, the ODR"},
    {"c_interop", "C headers, extern \"C\", wrapping a C library, name mangling, layout"},
    {"memory", "stack vs heap, RAII, smart pointers, rule of 0/3/5, move semantics"},
    {"oop", "classes, constructors, inheritance, vtables, slicing, diamonds, CRTP, PIMPL"},
    {"collections", "containers, iterators, algorithms, ranges and views, span"},
    {"templates", "function/class templates, specialisation, variadics, concepts, SFINAE"},
    {"exceptions", "try/catch, custom hierarchies, error codes, optional, noexcept"},
    {"io", "iostreams, string streams, files, std::filesystem, std::format"},
    {"datetime", "chrono durations and clocks, calendar dates, <ctime>, time zones"},
    {"concurrency", "threads, jthread, mutexes, condition variables, atomics, futures"},
    {"testing", "assert, static_assert, a tiny test harness, Calculator tests"},
    {"under_the_hood", "constexpr, variant, string_view traps, attributes, UB and sanitizers"},
};

int main()
{
    std::cout << "C++ Zero to Hero\n";
    std::cout << "Build the examples with CMake and run them one at a time.\n\n";

    int number = 1;
    for (const auto& topic : kSyllabus) {
        std::cout << (number < 10 ? " " : "") << number++ << ". " << topic.folder;
        for (auto pad = topic.folder.size(); pad < 16; ++pad) std::cout << ' ';
        std::cout << topic.summary << '\n';
    }

    std::cout << "\nExample: cmake --build build --target run_basics\n";
}
