// ============================================================
// Constructors in Depth
// ============================================================
// Member initialiser lists, the order members are really
// initialised in, delegating constructors, explicit, default
// member initialisers, and destruction order.
//
// Build and run:
//   cmake --build build --target oop_02_constructors
//   ./build/bin/oop_02_constructors
// ============================================================

#include <iostream>
#include <string>
#include <utility>

// ------------------------------------------------------------
// Member Initialiser Lists
// ------------------------------------------------------------
// Members are initialised BEFORE the constructor body runs.
// Assigning in the body means: default-construct, then assign.
// The initialiser list constructs each member once, directly.
// It is the only way to initialise const members, reference
// members and members with no default constructor.

class Temperature {
public:
    // Good: initialise in the list.
    explicit Temperature(double celsius) : celsius_{celsius}, unit_label_{"C"} {}

    double celsius() const { return celsius_; }
    const std::string& unit() const { return unit_label_; }

private:
    const double celsius_;      // const member: MUST be set in the initialiser list
    std::string unit_label_;
};

// ------------------------------------------------------------
// Initialisation Order Follows DECLARATION Order
// ------------------------------------------------------------
// Not the order written in the initialiser list. This compiles
// and is a real bug (GCC/Clang -Wreorder warns, in -Wall):
//
//   class Range {
//       int size_;
//       int start_, end_;
//   public:
//       Range(int s, int e) : start_{s}, end_{e}, size_{end_ - start_} {}
//       // size_ is declared first, so it is initialised FIRST,
//       // reading end_ and start_ before they hold any value.
//   };
//
// Rule: write the list in declaration order, and never let a
// member's initialiser read a member declared after it.

class Range {
public:
    Range(int start, int end) : start_{start}, end_{end}, size_{end - start} {}   // reads parameters, not members
    int start() const { return start_; }
    int end() const { return end_; }
    int size() const { return size_; }

private:
    int start_;
    int end_;
    int size_;
};

// ------------------------------------------------------------
// Default Member Initialisers and Delegating Constructors
// ------------------------------------------------------------
// Defaults written on the member are used by every constructor
// that does not mention that member.
// A delegating constructor (C++11) calls another constructor of
// the same class, so validation lives in one place. Same idea as
// this(...) in Java and : this(...) in C#.

class Connection {
public:
    Connection(std::string host, int port, int timeout_seconds)
        : host_{std::move(host)}, port_{port}, timeout_{timeout_seconds}
    {
        if (port_ <= 0 || port_ > 65535) port_ = 80;
        std::cout << "  main constructor: " << describe() << '\n';
    }

    explicit Connection(std::string host) : Connection{std::move(host), 443, 30}
    {
        std::cout << "  delegating constructor finished\n";
    }

    Connection() : Connection{"localhost"} {}

    std::string describe() const
    {
        return host_ + ":" + std::to_string(port_) + " (timeout " + std::to_string(timeout_) + "s"
               + (secure_ ? ", secure)" : ")");
    }

private:
    std::string host_;
    int port_;
    int timeout_;
    bool secure_ = true;   // default member initialiser
};

// ------------------------------------------------------------
// explicit and Converting Constructors
// ------------------------------------------------------------
// A constructor callable with ONE argument is also an implicit
// conversion. That allows surprising calls such as passing an int
// where a Distance was expected. Mark single-argument
// constructors explicit unless the conversion is genuinely natural
// (std::string from const char* is a good example of natural).

class Metres {
public:
    explicit Metres(double value) : value_{value} {}
    double value() const { return value_; }

private:
    double value_;
};

class Label {
public:
    Label(const char* text) : text_{text} {}   // implicit on purpose: "abc" -> Label is natural
    const std::string& text() const { return text_; }

private:
    std::string text_;
};

void print_metres(Metres m) { std::cout << "  " << m.value() << " m\n"; }
void print_label(const Label& l) { std::cout << "  label: " << l.text() << '\n'; }

// ------------------------------------------------------------
// Construction and Destruction Order of Members
// ------------------------------------------------------------
// Members are constructed in declaration order and destroyed in
// REVERSE order, after the destructor body has run.

struct Part {
    explicit Part(const char* name) : name_{name} { std::cout << "  + " << name_ << '\n'; }
    ~Part() { std::cout << "  - " << name_ << '\n'; }
    const char* name_;
};

class Machine {
public:
    Machine() { std::cout << "  Machine constructor body\n"; }
    ~Machine() { std::cout << "  Machine destructor body\n"; }

private:
    Part engine_{"engine"};
    Part wheels_{"wheels"};
};

int main()
{
    std::cout << "=== Initialiser Lists ===\n";
    Temperature t{21.5};
    std::cout << "  " << t.celsius() << ' ' << t.unit() << '\n';

    Range r{3, 10};
    std::cout << "  Range{" << r.start() << ", " << r.end() << "}.size() = " << r.size() << '\n';

    std::cout << "\n=== Delegating Constructors ===\n";
    Connection c;

    std::cout << "\n=== explicit ===\n";
    print_metres(Metres{5.0});
    // print_metres(5.0);          // error: Metres(double) is explicit
    print_label("implicit is fine here");

    std::cout << "\n=== Member Construction and Destruction Order ===\n";
    {
        Machine m;
    }
}
