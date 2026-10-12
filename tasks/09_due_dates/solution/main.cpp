// ============================================================
// Task 09: Loans with Due Dates (solution, Core + Stretch)
// ============================================================
// Session 9: dates (see datetime/02_calendar.cpp). Builds on
// task 08. "Today" is an input, never the real clock, so the
// output is the same whenever and wherever it runs.
//
// Build and run (from the repository root):
//   cmake --build build --target task09_solution
//   ./build/bin/task09_solution tasks/data [TODAY]             Core (checked)
//   ./build/bin/task09_solution tasks/data [TODAY] --stretch   Core, then Stretch
// DATA_DIR defaults to tasks/data, TODAY to 2026-10-12.
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 09 -- build/bin/task09_solution tasks/data
// ============================================================

#include "library.h"
#include "storage.h"

#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

constexpr std::string_view DEFAULT_TODAY = "2026-10-12";

void loans_table(const Library& library, Date today)
{
    std::cout << "Loans\n";
    std::cout << std::format("{:>2}  {:<30}  {:<12}  {:<10}  {:<10}  Status\n", "ID", "Book", "Member", "Borrowed", "Due");
    std::cout << std::format("{}  {}  {}  {}  {}  {}\n", std::string(2, '-'), std::string(30, '-'),
                             std::string(12, '-'), std::string(10, '-'), std::string(10, '-'), std::string(6, '-'));
    for (const Loan* loan : library.all_loans()) {
        std::cout << std::format("{:>2}  {:<30}  {:<12}  {}  {}  {}\n", loan->id, library.book(loan->book_id).title(),
                                 library.member(loan->member_id).name(), to_iso(loan->borrowed),
                                 to_iso(library.due_date(*loan)), library.status(*loan, today));
    }
}

void overdue_report(const Library& library, Date today)
{
    const auto overdue = library.overdue(today);
    std::cout << "Overdue on " << to_iso(today) << ": " << plural(overdue.size(), "loan", "loans") << '\n';
    for (const Loan* loan : overdue) {
        const Book& book = library.book(loan->book_id);
        const Member& member = library.member(loan->member_id);
        const auto days = static_cast<std::size_t>(library.days_late(*loan, today));
        std::cout << plural(days, "day", "days") << ": " << book.title() << ", " << member.name() << " ("
                  << member.type() << "), due " << to_iso(library.due_date(*loan)) << ", fine "
                  << format_money(library.fine(*loan, today)) << '\n';
    }
}

void fines_report(const Library& library, Date today)
{
    int on_overdue = 0;
    int on_returns = 0;
    for (const Loan* loan : library.all_loans()) {
        (loan->is_open() ? on_overdue : on_returns) += library.fine(*loan, today);
    }
    std::cout << "Fines\n";
    std::cout << "on overdue loans: " << format_money(on_overdue) << '\n';
    std::cout << "on late returns: " << format_money(on_returns) << '\n';
}

struct Action {
    std::string_view name;
    int member_id;
    int book_id;
};

void new_loans(Library& library, Date today)
{
    std::cout << "New loans on " << to_iso(today) << '\n';
    const std::vector<Action> actions{{"borrow", 105, 15}, {"borrow", 105, 16}, {"return", 102, 14}};
    for (const Action& action : actions) {
        try {
            if (action.name == "borrow") {
                const Loan& loan = library.borrow(action.member_id, action.book_id, today);
                std::cout << "OK: " << library.member(action.member_id).name() << " borrowed "
                          << library.book(action.book_id).title() << ", due " << to_iso(library.due_date(loan)) << '\n';
            } else {
                const Loan& loan = library.return_book(action.member_id, action.book_id, today);
                const int late = library.days_late(loan, today);
                const std::string tail =
                    late == 0 ? "on time"
                              : plural(static_cast<std::size_t>(late), "day", "days") + " late, fine " +
                                    format_money(library.fine(loan, today));
                std::cout << "OK: " << library.member(action.member_id).name() << " returned "
                          << library.book(action.book_id).title() << ", " << tail << '\n';
            }
        } catch (const LibraryError& error) {
            std::cout << "Error: " << error.what() << '\n';
        }
    }
}

// ------------------------------------------------------------
// Stretch: Closed on Sundays, and What Is Due This Week
// ------------------------------------------------------------

// A Sunday due date moves to the Monday
Date open_day(Date day)
{
    return std::chrono::weekday{day} == std::chrono::Sunday ? day + std::chrono::days{1} : day;
}

void stretch(const Library& library, Date today)
{
    std::cout << "Stretch\n";
    for (const Loan* loan : library.all_loans()) {
        const Date due = library.due_date(*loan);
        if (open_day(due) != due) {
            std::cout << "loan " << loan->id << ": due " << to_iso(due) << " is a Sunday, moves to "
                      << to_iso(open_day(due)) << '\n';
        }
    }
    const Date week_end = today + std::chrono::days{7};
    std::cout << "Due by " << to_iso(week_end) << ": ";
    bool first = true;
    for (const Loan* loan : library.all_loans()) {
        const Date due = library.due_date(*loan);
        if (loan->is_open() && today <= due && due <= week_end) {
            std::cout << (first ? "" : ", ") << library.book(loan->book_id).title();
            first = false;
        }
    }
    std::cout << '\n';
}

int run(int argc, char* argv[])
{
    std::vector<std::string> args;   // the arguments that are not --flags
    bool show_stretch = false;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--stretch") show_stretch = true;
        else if (!arg.starts_with("--")) args.emplace_back(arg);
    }
    const std::filesystem::path data = args.size() > 0 ? std::filesystem::path{args[0]} : "tasks/data";
    const std::optional<Date> today = parse_iso_date(args.size() > 1 ? args[1] : std::string(DEFAULT_TODAY));
    if (!today) {
        std::cerr << "today must be a date YYYY-MM-DD\n";
        return 2;
    }

    Library library = load_all(data);
    std::cout << "Today is " << to_iso(*today) << '\n';
    loans_table(library, *today);
    overdue_report(library, *today);
    fines_report(library, *today);
    new_loans(library, *today);
    if (show_stretch) {
        stretch(library, *today);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    // A missing data folder or a folder that cannot be written ends
    // the program with a message, not an unhandled exception
    try {
        return run(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
