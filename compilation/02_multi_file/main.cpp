// ============================================================
// Separate Compilation: Headers, Linkage and the ODR
// ============================================================
// This program is built from four .cpp files:
//
//   geometry.cpp  logger.cpp  inventory.cpp  -> static library
//   main.cpp                                  -> the executable
//
// Building by hand shows what CMake does for you:
//
//   g++ -std=c++20 -c geometry.cpp  -o geometry.o
//   g++ -std=c++20 -c logger.cpp    -o logger.o
//   g++ -std=c++20 -c inventory.cpp -o inventory.o
//   ar rcs libgeometry.a geometry.o logger.o inventory.o   (a static library is an archive of .o files)
//   g++ -std=c++20 -c main.cpp -o main.o
//   g++ main.o -L. -lgeometry -o multi_file                (link)
//
//   MSVC: cl /std:c++20 /EHsc /c geometry.cpp logger.cpp inventory.cpp
//         lib /OUT:geometry.lib geometry.obj logger.obj inventory.obj
//         cl /std:c++20 /EHsc main.cpp geometry.lib
//
// Change only logger.cpp and only logger.o is rebuilt, then
// everything is relinked. That is the point of separate files.
//
// Try these to meet the linker errors on purpose:
//   - delete the definition "int points_created = 0;" in geometry.cpp
//       -> undefined reference to geometry::points_created (LNK2001)
//   - remove "extern" from the declaration in geometry.h
//       -> multiple definition of geometry::points_created (LNK2005)
//   - remove "inline" from to_degrees in units.hpp
//       -> multiple definition of units::to_degrees
//
// Build and run:
//   cmake --build build --target compilation_02_multi_file
//   ./build/bin/compilation_02_multi_file
// ============================================================

#include "geometry.h"
#include "geometry.h"   // included twice on purpose: the include guard makes this harmless
#include "inventory.h"
#include "logger.h"
#include "units.hpp"

#include <iostream>

void use_library()
{
    std::cout << "=== Calling Functions Defined in Another File ===\n";

    auto a = geometry::make_point(0, 0);
    auto b = geometry::make_point(3, 4);

    std::cout << "distance(a, b)       = " << geometry::distance(a, b) << '\n';   // 5
    std::cout << "circle_area(1)       = " << geometry::circle_area(1) << '\n';
    std::cout << "to_degrees(pi)       = " << units::to_degrees(units::pi) << '\n';
    std::cout << "tau                  = " << units::tau << '\n';
    std::cout << "distance_call_count  = " << geometry::distance_call_count()
              << " (reads a static variable private to geometry.cpp)\n";
    std::cout << "points_created       = " << geometry::points_created
              << " (extern variable, defined in geometry.cpp)\n";
}

void show_linkage()
{
    std::cout << "\n=== Shared vs Per-File Objects ===\n";

    // inline variable: ONE object for the whole program.
    ++units::shared_counter;
    std::cout << "shared_counter (inline)  = " << units::shared_counter
              << "  (2 increments in geometry.cpp + 1 here)\n";
    std::cout << "same address in both files? " << std::boolalpha
              << (&units::shared_counter == geometry::shared_counter_address_from_library()) << '\n';

    // static variable in a header: one copy PER translation unit.
    ++units::per_file_counter;
    std::cout << "per_file_counter in main.cpp     = " << units::per_file_counter << '\n';       // 1
    std::cout << "per_file_counter in geometry.cpp = "
              << geometry::per_file_counter_from_library() << '\n';                                // 2
}

void forward_declarations()
{
    std::cout << "\n=== Forward Declarations ===\n";

    Logger logger{"inventory"};
    Inventory inventory{logger};
    inventory.add("bolts", 40);
    inventory.add("nuts", 25);
    inventory.add("bolts", 10);

    std::cout << "bolts in stock: " << inventory.quantity_of("bolts") << '\n';   // 50
    std::cout << "log lines     : " << logger.count() << '\n';                   // 3
}

int main()
{
    use_library();
    show_linkage();
    forward_declarations();
}
