// ============================================================
// Task 10: SQLite Storage (solution, Core + Stretch)
// ============================================================
// Session 10: databases, through SQLite's C API (see c_interop/
// for calling C from C++). Builds on task 09: the CSV files are
// imported once into out/library.db (rebuilt on every run from
// data/schema.sql), and from then on the database is the store.
//
// Build and run (from the repository root):
//   cmake --build build --target task10_solution
//   ./build/bin/task10_solution tasks/data [TODAY]             Core (checked)
//   ./build/bin/task10_solution tasks/data [TODAY] --stretch   Core, then Stretch
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 10 -- build/bin/task10_solution tasks/data
// ============================================================

#include "db.h"
#include "library.h"
#include "storage.h"

#include <cstddef>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

constexpr std::string_view DEFAULT_TODAY = "2026-10-12";

// Exactly the task 09 report, now fed from the database
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

struct Action {
    std::string_view name;
    int member_id;
    int book_id;
};

int run(int argc, char* argv[])
{
    std::vector<std::string> args;
    bool show_stretch = false;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--stretch") show_stretch = true;
        else if (!arg.starts_with("--")) args.emplace_back(arg);
    }
    const fs::path data = args.size() > 0 ? fs::path{args[0]} : "tasks/data";
    const std::optional<Date> today = parse_iso_date(args.size() > 1 ? args[1] : std::string(DEFAULT_TODAY));
    if (!today) {
        std::cerr << "today must be a date YYYY-MM-DD\n";
        return 2;
    }

    // --------------------------------------------------------
    // Core 1: a fresh database from schema.sql, filled from CSV
    // --------------------------------------------------------
    const fs::path db_path = fs::path{"out"} / "library.db";
    fs::create_directories(db_path.parent_path());
    fs::remove(db_path);   // start clean so every run is the same

    LibraryDb db{db_path};
    db.create(data / "schema.sql");
    std::cout << "Created out/library.db from schema.sql\n";

    LoadResult rows;
    load_books_csv(data / "books.csv", rows);
    load_members_csv(data / "members.csv", rows);
    load_loans_csv(data / "loans.csv", rows);
    db.import_rows(rows);
    std::cout << "Imported " << db.count("books") << " books, " << db.count("members") << " members, "
              << db.count("loans") << " loans\n";

    // --------------------------------------------------------
    // Core 2: questions answered in SQL
    // --------------------------------------------------------
    std::cout << "Books per genre\n";
    for (const GenreCount& row : db.genre_counts()) {
        std::cout << row.genre << ": " << row.count << '\n';
    }

    std::cout << "Nothing on the shelf\n";
    for (const ShelfRow& row : db.unavailable_books()) {
        std::cout << '#' << row.id << ' ' << row.title << ": " << row.available << " of " << row.copies
                  << " available\n";
    }

    for (std::string_view text : {"the", "x' OR '1'='1"}) {
        const auto found = db.search_titles(text);
        std::cout << "Search \"" << text << "\": " << plural(found.size(), "book", "books") << '\n';
        for (const TitleRow& row : found) {
            std::cout << "  #" << row.id << ' ' << row.title << " (" << row.year << ")\n";
        }
    }

    std::cout << "Open loans per member\n";
    for (const MemberLoans& row : db.open_loans_per_member()) {
        std::cout << '#' << row.id << ' ' << row.name << ": " << row.open_loans << '\n';
    }

    // --------------------------------------------------------
    // Core 3: the task 09 rules decide, the database remembers
    // --------------------------------------------------------
    Library library = db.load_library();
    std::cout << "New loans on " << to_iso(*today) << '\n';
    const std::vector<Action> actions{{"borrow", 105, 15}, {"borrow", 105, 16}, {"return", 102, 14}};
    for (const Action& action : actions) {
        try {
            const std::string& name = library.member(action.member_id).name();
            const std::string& title = library.book(action.book_id).title();
            if (action.name == "borrow") {
                const Loan& loan = library.borrow(action.member_id, action.book_id, *today);
                db.insert_loan(loan);
                std::cout << "OK: " << name << " borrowed " << title << ", due " << to_iso(library.due_date(loan))
                          << ", saved as loan " << loan.id << '\n';
            } else {
                const Loan& loan = library.return_book(action.member_id, action.book_id, *today);
                db.mark_returned(loan);
                std::cout << "OK: " << name << " returned " << title << ", fine "
                          << format_money(library.fine(loan, *today)) << ", saved\n";
            }
        } catch (const LibraryError& error) {
            std::cout << "Error: " << error.what() << '\n';
        }
    }

    // Read everything back from the database to prove it was saved
    const Library reloaded = db.load_library();
    int open_loans = 0;
    for (const Loan* loan : reloaded.all_loans()) {
        if (loan->is_open()) ++open_loans;
    }
    std::cout << "Reloaded: " << reloaded.book_count() << " books, " << reloaded.member_count() << " members, "
              << reloaded.loan_count() << " loans (" << open_loans << " open)\n";
    overdue_report(reloaded, *today);

    // --------------------------------------------------------
    // Stretch: a borrow that rolls back, and an index
    // --------------------------------------------------------
    if (show_stretch) {
        std::cout << "Stretch\n";
        db.add_member_index();
        const int before = db.count("loans");
        const std::optional<int> refused = db.borrow_in_transaction(101, 3, *today);
        std::cout << "borrow 101 3 in a transaction: " << (refused ? "OK" : "refused, rolled back") << '\n';
        std::cout << "loans before " << before << ", after " << db.count("loans") << '\n';
        const std::optional<int> saved = db.borrow_in_transaction(101, 4, *today);
        std::cout << "borrow 101 4 in a transaction: saved as loan " << saved.value_or(0) << '\n';
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
