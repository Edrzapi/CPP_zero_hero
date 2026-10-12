// ============================================================
// switch
// ============================================================
// switch works on integers, characters and enums only: not on
// strings (unlike Java, C# and Python's match). Cases fall
// through unless you break, exactly as in C.
//
// Build and run:
//   cmake --build build --target control_flow_02_switch
//   ./build/bin/control_flow_02_switch
// ============================================================

#include <iostream>
#include <string>
#include <string_view>

// ------------------------------------------------------------
// Basic switch and Fall-Through
// ------------------------------------------------------------
// Without break, execution carries on into the next case.
// Deliberate fall-through should be marked [[fallthrough]] so
// readers (and -Wimplicit-fallthrough) know it is intended.

void day_type(int day)   // 1 = Monday ... 7 = Sunday
{
    switch (day) {
    case 6:
    case 7:   // grouping labels: no statements in between, no attribute needed
        std::cout << day << ": weekend\n";
        break;
    case 1:
        std::cout << day << ": Monday, start of the week\n";
        [[fallthrough]];   // intended: Monday is also a weekday
    case 2:
    case 3:
    case 4:
    case 5:
        std::cout << day << ": weekday\n";
        break;
    default:
        std::cout << day << ": not a day\n";
        break;
    }
}

// ------------------------------------------------------------
// switch on enum class
// ------------------------------------------------------------
// Leave out "default" when switching over an enum class: the
// compiler then warns (-Wswitch, part of -Wall on GCC and Clang)
// if you forget an enumerator after the enum grows.

enum class Command { Start, Stop, Pause };

std::string_view describe(Command command)
{
    switch (command) {
    case Command::Start: return "starting";
    case Command::Stop:  return "stopping";
    case Command::Pause: return "pausing";
    }
    return "unknown command";
}

// ------------------------------------------------------------
// switch with an Initialiser and Scoped Cases (C++17)
// ------------------------------------------------------------
// Declaring a variable inside a case needs braces, otherwise
// a later case could jump past its initialisation (an error).

int parse_digit(char c) { return c - '0'; }

void switch_with_init(char input)
{
    switch (int digit = parse_digit(input); digit) {
    case 0: {
        std::string message = "zero";   // braces give this its own scope
        std::cout << input << " -> " << message << '\n';
        break;
    }
    case 1:
        std::cout << input << " -> one\n";
        break;
    default:
        std::cout << input << " -> digit value " << digit << '\n';
        break;
    }
}

// ------------------------------------------------------------
// Switching on Strings
// ------------------------------------------------------------
// Not allowed directly. Options: an if / else if chain, a map
// from string to handler, or hash the string first. The if
// chain is clearest for a handful of values.

void string_dispatch(std::string_view cmd)
{
    if (cmd == "help") {
        std::cout << cmd << " -> show help\n";
    } else if (cmd == "quit") {
        std::cout << cmd << " -> quit\n";
    } else {
        std::cout << cmd << " -> unknown\n";
    }
}

int main()
{
    std::cout << "=== switch and Fall-Through ===\n";
    day_type(1);
    day_type(3);
    day_type(7);
    day_type(9);

    std::cout << "\n=== switch on enum class ===\n";
    std::cout << describe(Command::Pause) << '\n';

    std::cout << "\n=== switch (init; value) ===\n";
    switch_with_init('0');
    switch_with_init('1');
    switch_with_init('7');

    std::cout << "\n=== Strings Need if/else ===\n";
    string_dispatch("help");
    string_dispatch("dance");
}
