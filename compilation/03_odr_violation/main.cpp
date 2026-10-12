// ============================================================
// ODR Violation (Opt-In, Deliberately Ill-Formed)
// ============================================================
// NOT built by default. Build with:
//   cmake -S . -B build -DCPPZH_BUILD_UB_DEMOS=ON
//   cmake --build build --target compilation_03_odr_violation
//   ./build/bin/compilation_03_odr_violation
//
// first.cpp and second.cpp each define an INLINE function called
// magic_number(), with DIFFERENT bodies. inline tells the linker
// "every definition is identical, keep any one of them". They
// are not identical, so the program is ill-formed, and the
// standard says "no diagnostic required": the compiler and
// linker are allowed to accept it silently.
//
// What usually happens: the linker keeps ONE copy (often the one
// from the first object file it sees) and BOTH files call it.
// Change the order of the sources in CMakeLists.txt, or turn on
// optimisation (which may inline each copy locally), and the
// output changes. That is what undefined behaviour looks like in
// practice: no crash, just quietly wrong answers.
//
// The same thing happens with two different class definitions
// sharing a name in different .cpp files (e.g. two "struct Node"
// helpers). Fix: put file-private helpers in an unnamed namespace.
//
// Detection: GCC/Clang with -flto can report -Wodr; ASan has
// detect_odr_violation for globals; MSVC /LTCG sometimes notices.
// ============================================================

#include <iostream>

int value_seen_by_first();
int value_seen_by_second();

int main()
{
    std::cout << "=== Two Different inline magic_number() Definitions ===\n";
    std::cout << "first.cpp  expects 1, sees " << value_seen_by_first() << '\n';
    std::cout << "second.cpp expects 2, sees " << value_seen_by_second() << '\n';
    std::cout << "At least one line is usually wrong, and the linker said nothing.\n";
}
