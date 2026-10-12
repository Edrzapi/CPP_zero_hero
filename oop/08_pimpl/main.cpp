// ============================================================
// PIMPL: Hiding the Implementation to Speed Up Builds
// ============================================================
// main.cpp includes widget.h, which mentions no <map>, no
// <vector> and no private members. widget.cpp is compiled into a
// separate static library (see oop/CMakeLists.txt).
//
// Try it: add a member to Widget::Impl in widget.cpp and rebuild
// with "cmake --build build --target oop_08_pimpl -v". Only
// widget.cpp is recompiled; main.cpp is not.
//
// Build and run:
//   cmake --build build --target oop_08_pimpl
//   ./build/bin/oop_08_pimpl
// ============================================================

#include "widget.h"

#include <iostream>
#include <utility>

int main()
{
    std::cout << "=== Using a PIMPL Class ===\n";

    Widget w{"gearbox"};
    w.add_tag("mechanical");
    w.add_tag("heavy");
    std::cout << "  " << w.describe() << '\n';

    Widget copy = w;            // deep copy through Impl's copy constructor
    copy.add_tag("spare");
    std::cout << "  copy:     " << copy.describe() << '\n';
    std::cout << "  original: " << w.describe() << '\n';

    Widget moved = std::move(copy);
    std::cout << "  moved:    " << moved.describe() << '\n';

    std::cout << "  sizeof(Widget) = " << sizeof(Widget)
              << " (just the pointer, whatever Impl contains)\n";
}
