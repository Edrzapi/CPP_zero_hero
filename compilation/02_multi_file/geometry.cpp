// ============================================================
// geometry.cpp: Definitions for the Geometry Library
// ============================================================
// Compiled on its own into geometry.o / geometry.obj. Including
// our own header first proves the header is self-contained (it
// compiles without relying on something included before it).
// ============================================================

#include "geometry.h"
#include "units.hpp"

#include <cmath>

namespace geometry {

// The ONE definition of the variable declared extern in the header.
int points_created = 0;

// ------------------------------------------------------------
// Internal Linkage: Helpers Private to This File
// ------------------------------------------------------------
// An unnamed (anonymous) namespace gives everything inside it
// internal linkage: invisible to the linker, so another .cpp can
// have its own square() without a clash. This is the C++ way.
// "static" on a function does the same thing; that is the C way.

namespace {

double square(double value) { return value * value; }

}  // namespace

static int distance_calls = 0;   // C-style internal linkage

double distance(Point a, Point b)
{
    ++distance_calls;
    return std::sqrt(square(b.x - a.x) + square(b.y - a.y));
}

double circle_area(double radius) { return units::pi * square(radius); }

Point make_point(double x, double y)
{
    ++points_created;
    ++units::shared_counter;     // the one shared object
    ++units::per_file_counter;   // THIS file's private copy
    return Point{x, y};
}

int* shared_counter_address_from_library() { return &units::shared_counter; }

int per_file_counter_from_library() { return units::per_file_counter; }

int distance_call_count() { return distance_calls; }

}  // namespace geometry
