// ============================================================
// units.hpp: Header-Only Constants and Helpers
// ============================================================
// Everything here is defined in the header itself, which is
// allowed because each item is either a constant, inline, or
// deliberately given internal linkage. The .hpp extension is
// just a convention some teams use for C++-only headers.
// ============================================================

// ------------------------------------------------------------
// #pragma once
// ------------------------------------------------------------
// Not in the ISO standard, but supported by GCC, Clang and MSVC.
// Shorter than include guards and immune to copy-paste mistakes
// in the guard name. It can be fooled by the same file reached
// through two different paths (symlinks, network shares), which
// is the main argument for classic guards in large codebases.

#pragma once

namespace units {

// ------------------------------------------------------------
// Linkage
// ------------------------------------------------------------
// External linkage: one object shared by the whole program.
// Internal linkage: each translation unit gets its own copy.
//
// - const / constexpr variables at namespace scope have INTERNAL
//   linkage by default (unlike C, where const globals are external).
//   Fine for constants: every .cpp gets its own copy of pi.
constexpr double pi = 3.14159265358979323846;

// - inline variables (C++17) have EXTERNAL linkage and may be
//   defined in a header: every .cpp sees the SAME object.
inline constexpr double tau = 2 * pi;
inline int shared_counter = 0;

// - static at namespace scope means INTERNAL linkage (C heritage).
//   Every .cpp that includes this header gets a SEPARATE
//   per_file_counter. Usually a bug in a header; shown on purpose.
static int per_file_counter = 0;

// inline functions: may be defined in every translation unit.
inline double to_degrees(double radians) { return radians * 180.0 / pi; }

}  // namespace units
