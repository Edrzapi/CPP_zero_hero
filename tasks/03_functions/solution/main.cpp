// ============================================================
// Task 03: Functions and a Menu (solution, Core + Stretch)
// ============================================================
// Session 3: functions (see functions/ and compilation/).
// Builds on task 02. The features live in shelf.h / shelf.cpp
// (Stretch 1); this file only runs the menu and prints.
//
// Build and run:
//   cmake --build build --target task03_solution
//   ./build/bin/task03_solution < tasks/data/input_03.txt
//   ./build/bin/task03_solution --stretch < tasks/data/input_03.txt
// Type the choices yourself if you run it without "<".
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 03 -- build/bin/task03_solution
// ============================================================

#include "shelf.h"

#include <iostream>
#include <optional>
#include <string>
#include <string_view>

constexpr std::string_view MENU = "Menu: 1 catalogue | 2 search | 3 report | 4 stats | 0 quit";

// Spaces and tabs off both ends ("\r" too, for a file saved on
// Windows and read on Linux).
std::string trim(const std::string& text)
{
    const auto first = text.find_first_not_of(" \t\r");
    if (first == std::string::npos) return "";
    const auto last = text.find_last_not_of(" \t\r");
    return text.substr(first, last - first + 1);
}

// Reads one line and echoes it as "> text", so the output reads
// like a conversation even when the input comes from a file.
// Returns std::nullopt at the end of the input.
std::optional<std::string> read_line()
{
    std::string line;
    if (!std::getline(std::cin, line)) {
        return std::nullopt;
    }
    std::string text = trim(line);
    std::cout << (text.empty() ? ">" : "> " + text) << '\n';   // never a trailing space
    return text;
}

void run_menu(bool include_stretch)
{
    std::cout << "Library Manager\n";
    while (true) {
        std::cout << MENU << '\n';
        const std::optional<std::string> choice = read_line();
        if (!choice || *choice == "0") {   // end of input counts as 0
            break;
        }

        // The choice is compared as text, so "abc" is just another
        // unknown choice: no number parsing, nothing to go wrong.
        // (switch only works on integers, hence the if chain.)
        if (*choice == "1") {
            std::cout << catalogue();
        } else if (*choice == "2") {
            std::cout << "Search for:\n";
            const std::optional<std::string> term = read_line();
            if (!term) break;
            std::cout << search(*term);
        } else if (*choice == "3") {
            std::cout << report();
        } else if (*choice == "4") {
            std::cout << stats(include_stretch);
        } else {
            std::cout << "Unknown choice: " << *choice << '\n';
        }
    }
    std::cout << "Goodbye\n";
}

int main(int argc, char* argv[])
{
    const bool include_stretch = argc > 1 && std::string_view{argv[1]} == "--stretch";
    run_menu(include_stretch);
}
