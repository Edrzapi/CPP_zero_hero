// ============================================================
// Copy vs Move Semantics
// ============================================================
// A copy duplicates an object's resources. A move TRANSFERS them,
// leaving the source valid but unspecified (usually empty). For a
// std::vector of a million elements, a copy allocates and copies a
// million ints; a move swaps three pointers.
//
// Build and run:
//   cmake --build build --target memory_05_move_semantics
//   ./build/bin/memory_05_move_semantics
// ============================================================

#include <iostream>
#include <string>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// Value Categories in One Paragraph
// ------------------------------------------------------------
// lvalue:  has a name / an identity you can take the address of
//          (a variable, *ptr, v[0]). Binds to T&.
// rvalue:  a temporary about to die (42, a + b, make_string()),
//          or an object you have marked with std::move. Binds to T&&.
// T&& in a function parameter means "I accept objects whose
// resources may be stolen". std::move does NOT move anything: it
// is a cast to T&& that says "you may steal from this".

void which(const std::string&) { std::cout << "  lvalue overload (copy)\n"; }
void which(std::string&&) { std::cout << "  rvalue overload (move)\n"; }

void value_categories()
{
    std::cout << "=== Value Categories ===\n";
    std::string name = "Ada";
    which(name);                    // name is an lvalue
    which(std::string{"temp"});     // a temporary is an rvalue
    which(std::move(name));         // std::move makes name an rvalue
    which(name + "!");              // the result of + is a temporary
}

// ------------------------------------------------------------
// A Class That Reports Copies and Moves
// ------------------------------------------------------------

class Tracked {
public:
    explicit Tracked(std::string label) : label_{std::move(label)} {}

    Tracked(const Tracked& other) : label_{other.label_}
    {
        std::cout << "  COPY " << label_ << '\n';
    }
    Tracked(Tracked&& other) noexcept : label_{std::move(other.label_)}
    {
        std::cout << "  MOVE " << label_ << '\n';
    }
    Tracked& operator=(const Tracked& other)
    {
        label_ = other.label_;
        std::cout << "  COPY-ASSIGN " << label_ << '\n';
        return *this;
    }
    Tracked& operator=(Tracked&& other) noexcept
    {
        label_ = std::move(other.label_);
        std::cout << "  MOVE-ASSIGN " << label_ << '\n';
        return *this;
    }
    ~Tracked() = default;

    const std::string& label() const { return label_; }

private:
    std::string label_;
};

void copy_vs_move()
{
    std::cout << "\n=== Copy vs Move ===\n";

    Tracked a{"a"};
    Tracked b = a;               // copy: a still needed afterwards
    Tracked c = std::move(a);    // move: a's string is stolen
    std::cout << "  after move, a.label() = \"" << a.label() << "\" (valid but unspecified)\n";
    // Only assign to or destroy a moved-from object; do not rely on its value.
    a = Tracked{"a again"};      // move-assign from a temporary: a is usable again
    std::cout << "  b = " << b.label() << ", c = " << c.label() << '\n';
}

// ------------------------------------------------------------
// noexcept Moves and std::vector
// ------------------------------------------------------------
// When a vector grows it must transfer its elements to new memory.
// It only MOVES them if the move constructor is noexcept;
// otherwise it COPIES, to keep the strong exception guarantee.
// Forgetting noexcept on a move constructor silently turns every
// reallocation into a full copy.

void vector_growth()
{
    std::cout << "\n=== vector Reallocation Uses noexcept Moves ===\n";
    std::vector<Tracked> items;
    items.reserve(1);
    items.emplace_back("first");          // constructed in place: no copy, no move
    std::cout << "  -- push second, forcing a reallocation:\n";
    items.emplace_back("second");         // "first" is MOVED into the new block
}

// ------------------------------------------------------------
// Move-Only Types
// ------------------------------------------------------------
// Some things must have exactly one owner: std::unique_ptr,
// std::thread, std::fstream, std::jthread. They delete their copy
// operations and provide moves. Store them in containers and
// return them from functions freely; pass them with std::move.

class Ticket {
public:
    explicit Ticket(int id) : id_{id} {}
    Ticket(const Ticket&) = delete;
    Ticket& operator=(const Ticket&) = delete;
    Ticket(Ticket&& other) noexcept : id_{std::exchange(other.id_, 0)} {}
    Ticket& operator=(Ticket&& other) noexcept
    {
        id_ = std::exchange(other.id_, 0);
        return *this;
    }
    int id() const { return id_; }

private:
    int id_;
};

Ticket issue(int id) { return Ticket{id}; }   // returning a move-only type: fine

void move_only()
{
    std::cout << "\n=== Move-Only Types ===\n";
    Ticket t = issue(7);
    std::vector<Ticket> queue;
    queue.push_back(std::move(t));
    queue.push_back(issue(8));
    std::cout << "  queued tickets: " << queue[0].id() << ", " << queue[1].id()
              << "; t.id() after move = " << t.id() << '\n';
}

// ------------------------------------------------------------
// Common Mistakes
// ------------------------------------------------------------
// return std::move(local);   prevents copy elision; just "return local;"
//                            (GCC -Wpessimizing-move, Clang too)
// const T x; std::move(x);   a const object cannot be moved from:
//                            this silently COPIES
// use after move             reading a moved-from object's value

int main()
{
    value_categories();
    copy_vs_move();
    vector_growth();
    move_only();
}
