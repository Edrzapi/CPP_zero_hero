// ============================================================
// widget.h: The Public Face of a PIMPL Class
// ============================================================
// PIMPL ("pointer to implementation") moves every private data
// member into a struct that is only DEFINED in widget.cpp. This
// header shows nothing but the public interface and one pointer.
//
// Why bother:
//   - Build times: adding, removing or changing a private member
//     touches only widget.cpp. Without PIMPL, every file that
//     includes widget.h recompiles.
//   - Hidden dependencies: the heavy headers the implementation
//     needs (<map>, <vector>, third-party libraries) are included
//     in widget.cpp only, not dragged into every user's build.
//   - Stable ABI: sizeof(Widget) never changes (one pointer), so a
//     shared library can change internals without breaking
//     programs compiled against an older version. Qt uses this
//     ("d-pointer") throughout.
//
// Costs: one heap allocation per object and one extra pointer
// hop per call; no inlining of member functions across the
// boundary.
// ============================================================

#ifndef CPPZH_WIDGET_H
#define CPPZH_WIDGET_H

#include <memory>
#include <string>

class Widget {
public:
    explicit Widget(std::string name);

    // The special members must be DECLARED here and DEFINED in
    // widget.cpp, where Impl is a complete type. If the compiler
    // generated ~Widget() here, unique_ptr<Impl> would try to
    // delete an incomplete type: a compile error ("can't delete an
    // incomplete type" / "use of undefined type").
    ~Widget();
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;
    Widget(const Widget& other);              // deep copy
    Widget& operator=(const Widget& other);

    void add_tag(const std::string& tag);
    std::string describe() const;

private:
    struct Impl;                    // forward declaration only
    std::unique_ptr<Impl> impl_;    // the only data member
};

#endif  // CPPZH_WIDGET_H
