// ============================================================
// Worked Example: Grading
// ============================================================
// Puts the control flow pieces together: validation first,
// then a ladder of conditions, then a switch on the result.
// The same example appears in Java Zero to Hero (GradingRunner).
//
// Build and run:
//   cmake --build build --target control_flow_03_grading
//   ./build/bin/control_flow_03_grading
// ============================================================

#include <array>
#include <iostream>
#include <optional>

// ------------------------------------------------------------
// Turning a Mark into a Grade
// ------------------------------------------------------------
// std::optional<char> says "a grade, or nothing" without a magic
// value like '?' or -1. Invalid marks return std::nullopt.
// See exceptions/03_error_codes_and_optional.cpp for more.

std::optional<char> grade_for(int mark)
{
    if (mark < 0 || mark > 100) {
        return std::nullopt;   // guard clause: reject bad input first
    }
    if (mark >= 70) return 'A';
    if (mark >= 60) return 'B';
    if (mark >= 50) return 'C';
    if (mark >= 40) return 'D';
    return 'F';
}

// ------------------------------------------------------------
// Acting on the Grade
// ------------------------------------------------------------

const char* comment_for(char grade)
{
    switch (grade) {
    case 'A': return "distinction";
    case 'B': return "merit";
    case 'C':
    case 'D': return "pass";
    default:  return "fail";
    }
}

int main()
{
    std::cout << "=== Grading ===\n";

    const std::array marks{85, 64, 52, 41, 12, 101, -5};

    for (int mark : marks) {
        if (auto grade = grade_for(mark)) {        // optional converts to bool
            std::cout << mark << " -> " << *grade << " (" << comment_for(*grade) << ")\n";
        } else {
            std::cout << mark << " -> invalid mark\n";
        }
    }
}
