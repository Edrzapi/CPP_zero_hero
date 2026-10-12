// ============================================================
// noexcept and the Exception Safety Guarantees
// ============================================================
// noexcept is a promise that a function will not throw. The
// compiler and library use it: std::vector only moves elements
// during reallocation if the move constructor is noexcept.
// If a noexcept function does throw, std::terminate is called.
//
// The exception safety guarantees (David Abrahams) describe what
// a function promises about state if an exception occurs:
//   nothrow   never throws (destructors, swap, moves)
//   strong    commit-or-rollback: on failure, nothing changed
//   basic     on failure, no leaks and invariants hold, but the
//             state may have changed
//   none      anything can happen. Avoid.
//
// Build and run:
//   cmake --build build --target exceptions_04_noexcept_and_safety
//   ./build/bin/exceptions_04_noexcept_and_safety
// ============================================================

#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// noexcept as a Specifier and as an Operator
// ------------------------------------------------------------
// noexcept            the function promises not to throw
// noexcept(expr)      conditional: noexcept if expr is true
// noexcept(call)      the OPERATOR: true if call is declared noexcept
//                     (checked at compile time, nothing is run)

int add(int a, int b) noexcept { return a + b; }
int risky(int a) { if (a < 0) throw std::invalid_argument{"negative"}; return a; }

template <typename T>
void swap_values(T& a, T& b) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                      std::is_nothrow_move_assignable_v<T>)
{
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

void noexcept_operator()
{
    std::cout << "=== noexcept ===\n" << std::boolalpha;
    std::cout << "  noexcept(add(1, 2))   = " << noexcept(add(1, 2)) << '\n';    // true
    std::cout << "  noexcept(risky(1))    = " << noexcept(risky(1)) << '\n';     // false

    int x = 1;
    int y = 2;
    std::cout << "  swap_values<int> noexcept? " << noexcept(swap_values(x, y)) << '\n';
    swap_values(x, y);
    std::cout << "  after swap_values: x = " << x << ", y = " << y << '\n';

    // Destructors are implicitly noexcept.
    std::cout << "  std::string destructor noexcept? "
              << std::is_nothrow_destructible_v<std::string> << '\n';
}

// ------------------------------------------------------------
// Why noexcept Moves Matter
// ------------------------------------------------------------

struct NoexceptMove {
    NoexceptMove() = default;
    NoexceptMove(const NoexceptMove&) { std::cout << "copy "; }
    NoexceptMove(NoexceptMove&&) noexcept { std::cout << "move "; }
};

struct ThrowingMove {
    ThrowingMove() = default;
    ThrowingMove(const ThrowingMove&) { std::cout << "copy "; }
    ThrowingMove(ThrowingMove&&) { std::cout << "move "; }   // not noexcept
};

template <typename T>
void grow(const char* label)
{
    std::vector<T> v(2);
    v.shrink_to_fit();
    std::cout << "  " << label << " on reallocation: ";
    v.emplace_back();   // forces the 2 existing elements to new memory
    std::cout << '\n';
}

// ------------------------------------------------------------
// The Strong Guarantee: Do the Risky Work on the Side
// ------------------------------------------------------------
// Pattern: build the new state in temporaries (which may throw),
// then commit with operations that cannot throw (swap, move).
// If anything throws before the commit, the object is unchanged.

class Roster {
public:
    // BASIC guarantee only: if validation fails half-way, some names
    // were already added.
    void add_all_basic(const std::vector<std::string>& names)
    {
        for (const auto& n : names) {
            validate(n);
            names_.push_back(n);
        }
    }

    // STRONG guarantee: all or nothing.
    void add_all_strong(const std::vector<std::string>& names)
    {
        std::vector<std::string> updated = names_;   // copy: may throw, harmless
        for (const auto& n : names) {
            validate(n);                             // may throw, harmless
            updated.push_back(n);
        }
        names_.swap(updated);                        // commit: noexcept
    }

    std::size_t size() const noexcept { return names_.size(); }

private:
    static void validate(const std::string& name)
    {
        if (name.empty()) throw std::invalid_argument{"empty name"};
    }

    std::vector<std::string> names_;
};

void guarantees()
{
    std::cout << "\n=== Basic vs Strong Guarantee ===\n";
    const std::vector<std::string> batch{"Ada", "Alan", "", "Grace"};

    Roster basic;
    try {
        basic.add_all_basic(batch);
    } catch (const std::invalid_argument&) {
        std::cout << "  basic:  failed, but size is now " << basic.size() << " (partially applied)\n";
    }

    Roster strong;
    try {
        strong.add_all_strong(batch);
    } catch (const std::invalid_argument&) {
        std::cout << "  strong: failed, size is still " << strong.size() << " (rolled back)\n";
    }
}

// ------------------------------------------------------------
// When to Write noexcept
// ------------------------------------------------------------
// - move constructors, move assignment and swap: almost always
// - destructors: already implicit
// - simple getters and leaf functions that truly cannot fail
// Do not sprinkle it everywhere: removing noexcept later is a
// breaking change, and a wrong noexcept turns an exception into
// std::terminate.

int main()
{
    noexcept_operator();

    std::cout << "\n=== std::vector Picks Move Only If noexcept ===\n";
    grow<NoexceptMove>("noexcept move");
    grow<ThrowingMove>("throwing move");

    guarantees();
}
