// ============================================================
// geometry.h: Declarations for the Geometry Library
// ============================================================
// A header holds DECLARATIONS that several .cpp files need to
// agree on. Every .cpp that includes it gets a textual copy, so
// a header must only contain things that may legally appear in
// many translation units:
//   - declarations of functions and extern variables
//   - class / struct / enum definitions
//   - inline functions and inline variables
//   - templates
//   - constexpr constants
// Ordinary (non-inline) function and variable DEFINITIONS go in
// exactly one .cpp file. That is the One Definition Rule (ODR).
// ============================================================

// ------------------------------------------------------------
// Include Guards
// ------------------------------------------------------------
// If a .cpp includes this header twice (directly, or via two
// other headers), the struct below would be defined twice: an
// error. The guard makes the second inclusion empty.
// The macro name must be unique across the whole project.
// units.hpp shows the alternative, #pragma once.

#ifndef CPPZH_GEOMETRY_H
#define CPPZH_GEOMETRY_H

namespace geometry {

// A class definition may appear in every translation unit, as
// long as every copy is token-for-token identical.
struct Point {
    double x;
    double y;
};

// Function DECLARATIONS: no body. The definitions are in geometry.cpp.
double distance(Point a, Point b);
double circle_area(double radius);
Point make_point(double x, double y);

// ------------------------------------------------------------
// extern Variables
// ------------------------------------------------------------
// "extern" declares a variable without defining it: "this exists
// somewhere else". geometry.cpp holds the single definition.
// Without extern this line would DEFINE a variable in every .cpp
// that includes the header, and the linker would report
// "multiple definition" (GCC) or LNK2005 (MSVC).

extern int points_created;

// Helpers used by main.cpp to show which objects are shared
// between translation units and which are per-file copies.
int* shared_counter_address_from_library();
int per_file_counter_from_library();
int distance_call_count();

}  // namespace geometry

#endif  // CPPZH_GEOMETRY_H
