// ============================================================
// Copy Elision, RVO and NRVO
// ============================================================
// Returning a large object by value is normally free: the
// compiler builds the result directly in the caller's variable.
//
//   RVO   return of a temporary (return Widget{...};)
//         GUARANTEED since C++17: not even a move happens, and the
//         type does not need a copy or move constructor at all.
//   NRVO  return of a named local (Widget w; ...; return w;)
//         allowed but not guaranteed. Every mainstream compiler does
//         it when there is a single named object returned on every
//         path. If NRVO is not possible, the local is MOVED (C++11),
//         not copied.
//
// Build and run (try both Debug and Release builds):
//   cmake --build build --target under_the_hood_08_copy_elision
//   ./build/bin/under_the_hood_08_copy_elision
// ============================================================

#include <iostream>
#include <string>
#include <utility>

class Tracer {
public:
    explicit Tracer(std::string name) : name_{std::move(name)} { std::cout << "    construct " << name_ << '\n'; }
    Tracer(const Tracer& other) : name_{other.name_} { std::cout << "    COPY " << name_ << '\n'; }
    Tracer(Tracer&& other) noexcept : name_{std::move(other.name_)} { std::cout << "    MOVE " << name_ << '\n'; }
    Tracer& operator=(const Tracer&) = default;
    Tracer& operator=(Tracer&&) noexcept = default;
    ~Tracer() = default;

    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// ------------------------------------------------------------
// Guaranteed Elision (C++17)
// ------------------------------------------------------------

Tracer make_temporary() { return Tracer{"prvalue"}; }

// A type that cannot be copied OR moved can still be returned by
// value, because there is nothing to copy: the object is created
// in its final place.
struct Pinned {
    int value;
    explicit Pinned(int v) : value{v} {}
    Pinned(const Pinned&) = delete;
    Pinned(Pinned&&) = delete;
};

Pinned make_pinned() { return Pinned{42}; }   // compiles only because of guaranteed elision

// ------------------------------------------------------------
// NRVO
// ------------------------------------------------------------

Tracer make_named()
{
    Tracer result{"named"};
    // ... work on result ...
    return result;            // NRVO: usually no copy and no move
}

// Two different named objects returned on different paths: the
// compiler cannot construct both in the caller's slot, so NRVO is
// usually lost and the chosen local is MOVED instead.
Tracer make_either(bool first)
{
    Tracer a{"either-a"};
    Tracer b{"either-b"};
    if (first) {
        return a;
    }
    return b;
}

// Returning a function PARAMETER is never elided: it is moved.
Tracer pass_through(Tracer t) { return t; }

// ------------------------------------------------------------
// Do Not std::move the Return Value
// ------------------------------------------------------------
//   Tracer bad() { Tracer t{"x"}; return std::move(t); }
// std::move turns the name into an expression that is no longer
// eligible for NRVO, forcing a move that would otherwise have been
// elided. GCC and Clang warn (-Wpessimizing-move).

int main()
{
    std::cout << "=== RVO (guaranteed) ===\n";
    Tracer a = make_temporary();
    Tracer b = Tracer{Tracer{"nested temporaries"}};   // still just one construction
    std::cout << "  got " << a.name() << " and " << b.name() << '\n';

    Pinned p = make_pinned();
    std::cout << "  non-movable Pinned returned by value: " << p.value << '\n';

    std::cout << "\n=== NRVO (usual, not guaranteed) ===\n";
    Tracer c = make_named();
    std::cout << "  got " << c.name() << '\n';

    std::cout << "\n=== NRVO Defeated: Two Candidates ===\n";
    Tracer d = make_either(false);
    std::cout << "  got " << d.name() << '\n';

    std::cout << "\n=== Returning a Parameter ===\n";
    Tracer e = pass_through(Tracer{"param"});   // argument elided, return moved
    std::cout << "  got " << e.name() << '\n';

    std::cout << "\n=== Passing an lvalue by Value Copies ===\n";
    Tracer f = pass_through(c);                 // copy into the parameter, then move out
    std::cout << "  got " << f.name() << '\n';
}
