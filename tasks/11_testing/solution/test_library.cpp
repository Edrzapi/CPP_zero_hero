// ============================================================
// Task 11: Test the Rules (solution, Core + Stretch)
// ============================================================
// Session 11: testing, with the repository's own harness,
// testing/mini_test.h (see testing/calculator_test.cpp). Every
// test builds a fresh library from the fixture, so no test
// depends on another one or on the order they run in. Dates are
// fixed values: nothing reads the real clock.
//
// Build and run:
//   cmake --build build --target task11_solution
//   ./build/bin/task11_solution               every test, by name
//   ./build/bin/task11_solution reserve       tests whose name contains "reserve"
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 11 -- build/bin/task11_solution
// ============================================================

#include "library.h"
#include "mini_test.h"
#include "storage.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// ------------------------------------------------------------
// The Fixture: Books 1 to 8 and Members 101 to 103, No Loans
// ------------------------------------------------------------

constexpr int AMIRA = 101;   // standard
constexpr int BEN = 102;     // student
constexpr int CHLOE = 103;   // staff

constexpr std::array<std::string_view, 8> FIXTURE_BOOKS{
    "1,Pride and Prejudice,Jane Austen,1813,Classic,3,print,432",
    "2,Nineteen Eighty-Four,George Orwell,1949,Dystopian,2,print,328",
    "3,The Hobbit,J. R. R. Tolkien,1937,Fantasy,0,print,310",
    "4,Dune,Frank Herbert,1965,Science Fiction,4,print,412",
    "5,Robinson Crusoe,Daniel Defoe,1719,Adventure,1,print,320",
    "6,The Time Machine,H. G. Wells,1895,Science Fiction,2,ebook,118",
    "7,Animal Farm,George Orwell,1945,Dystopian,5,print,112",
    "8,Emma,Jane Austen,1815,Classic,1,audio,1150",
};

std::vector<std::string> split(std::string_view line)
{
    std::istringstream stream{std::string{line}};
    std::vector<std::string> fields;
    std::string field;
    while (std::getline(stream, field, ',')) fields.push_back(field);
    return fields;
}

// A fresh library for every test: the C++ version of setUp().
// It is returned by value, so each test owns its own copy.
Library fixture()
{
    Library library;
    for (std::string_view line : FIXTURE_BOOKS) {
        library.add_book(make_book(split(line)));
    }
    library.add_member(Member{AMIRA, "Amira Khan", "amira.khan@example.com", "standard", "2023-01-15"});
    library.add_member(Member{BEN, "Ben Taylor", "ben.taylor@example.com", "student", "2024-09-02"});
    library.add_member(Member{CHLOE, "Chloe Evans", "chloe.evans@example.com", "staff", "2019-05-20"});
    return library;
}

// A date from numbers. C++20 can also write 2026y / 10 / 12.
Date day(int y, unsigned m, unsigned d)
{
    return Date{std::chrono::year{y} / std::chrono::month{m} / std::chrono::day{d}};
}

const Date TODAY = day(2026, 10, 12);

// A loan built directly, for the date tests
Loan loan_of(int member_id, Date borrowed)
{
    return Loan{1, 1, member_id, borrowed, std::nullopt};
}

// ------------------------------------------------------------
// Borrowing
// ------------------------------------------------------------

TEST_CASE("borrow reduces available")
{
    Library library = fixture();
    library.borrow(AMIRA, 4, TODAY);
    CHECK_EQ(library.book(4).available(), 3);
}

TEST_CASE("return restores available")
{
    Library library = fixture();
    library.borrow(AMIRA, 4, TODAY);
    library.return_book(AMIRA, 4, TODAY);
    CHECK_EQ(library.book(4).available(), 4);
}

TEST_CASE("borrow unknown book raises")
{
    Library library = fixture();
    CHECK_THROWS(library.borrow(AMIRA, 99, TODAY), UnknownBookError);
}

TEST_CASE("borrow unknown member raises")
{
    Library library = fixture();
    CHECK_THROWS(library.borrow(999, 1, TODAY), UnknownMemberError);
}

TEST_CASE("borrow with no copies raises")
{
    Library library = fixture();
    CHECK_THROWS(library.borrow(AMIRA, 3, TODAY), BookNotAvailableError);
}

TEST_CASE("return book not on loan raises")
{
    Library library = fixture();
    CHECK_THROWS(library.return_book(AMIRA, 4, TODAY), NotOnLoanError);
}

TEST_CASE("standard limit is three loans")
{
    Library library = fixture();
    for (int book_id : {1, 2, 4}) {
        library.borrow(AMIRA, book_id, TODAY);
    }
    CHECK_THROWS(library.borrow(AMIRA, 7, TODAY), MemberLimitReachedError);
}

TEST_CASE("student limit is five loans")
{
    Library library = fixture();
    for (int book_id : {1, 2, 4, 6, 7}) {
        library.borrow(BEN, book_id, TODAY);
    }
    CHECK_THROWS(library.borrow(BEN, 8, TODAY), MemberLimitReachedError);
}

// ------------------------------------------------------------
// Dates and Fines
// ------------------------------------------------------------
// Dates are compared as YYYY-MM-DD text, so a failure prints two
// readable dates.

TEST_CASE("due date standard is 14 days")
{
    Library library = fixture();
    CHECK_EQ(to_iso(library.due_date(loan_of(AMIRA, day(2026, 10, 12)))), "2026-10-26");
}

TEST_CASE("due date student is 21 days")
{
    Library library = fixture();
    CHECK_EQ(to_iso(library.due_date(loan_of(BEN, day(2026, 10, 12)))), "2026-11-02");
}

TEST_CASE("due date staff is 28 days")
{
    Library library = fixture();
    CHECK_EQ(to_iso(library.due_date(loan_of(CHLOE, day(2026, 10, 12)))), "2026-11-09");
}

TEST_CASE("due date crosses month and year end")
{
    Library library = fixture();
    CHECK_EQ(to_iso(library.due_date(loan_of(BEN, day(2026, 12, 20)))), "2027-01-10");
}

TEST_CASE("due date in leap year")
{
    Library library = fixture();
    CHECK_EQ(to_iso(library.due_date(loan_of(AMIRA, day(2028, 2, 20)))), "2028-03-05");
}

TEST_CASE("not overdue on due date")
{
    Library library = fixture();
    const Loan loan = loan_of(AMIRA, day(2026, 9, 28));   // due 2026-10-12
    CHECK_EQ(library.days_late(loan, TODAY), 0);
    CHECK_EQ(library.fine(loan, TODAY), 0);
}

TEST_CASE("fine one day late standard")
{
    Library library = fixture();
    CHECK_EQ(library.fine(loan_of(AMIRA, day(2026, 9, 27)), TODAY), 25);   // due 2026-10-11
}

TEST_CASE("fine ten days late student")
{
    Library library = fixture();
    CHECK_EQ(library.fine(loan_of(BEN, day(2026, 9, 11)), TODAY), 100);   // due 2026-10-02
}

TEST_CASE("staff never fined")
{
    Library library = fixture();
    CHECK_EQ(library.fine(loan_of(CHLOE, day(2026, 1, 1)), TODAY), 0);
}

TEST_CASE("late return fine uses return date")
{
    // Due 2026-10-01, back 2026-10-05: 4 days late, whatever today is
    Library library = fixture();
    Loan loan = loan_of(AMIRA, day(2026, 9, 17));
    loan.returned = day(2026, 10, 5);
    CHECK_EQ(library.fine(loan, day(2026, 12, 25)), 100);
}

TEST_CASE("format money")
{
    CHECK_EQ(format_money(0), "£0.00");
    CHECK_EQ(format_money(5), "£0.05");
    CHECK_EQ(format_money(125), "£1.25");
    CHECK_EQ(format_money(1000), "£10.00");
}

// ------------------------------------------------------------
// Reservations (written first, watched fail, then made to pass)
// ------------------------------------------------------------
// In each one, Amira has first borrowed book 5, the only copy of
// Robinson Crusoe. A small fixture struct does that setup once.

struct ReservationFixture {
    Library library = fixture();
    ReservationFixture() { library.borrow(AMIRA, 5, TODAY); }
};

TEST_CASE("reserve book with no free copies joins queue")
{
    ReservationFixture f;
    CHECK_EQ(f.library.reserve(BEN, 5), 1);
    CHECK_EQ(f.library.reserve(CHLOE, 5), 2);
}

TEST_CASE("reserve book with free copies raises")
{
    ReservationFixture f;
    CHECK_THROWS(f.library.reserve(BEN, 1), ReservationNotNeededError);
}

TEST_CASE("reserve same book twice raises")
{
    ReservationFixture f;
    f.library.reserve(BEN, 5);
    CHECK_THROWS(f.library.reserve(BEN, 5), AlreadyReservedError);
}

TEST_CASE("returned copy is held for first in queue")
{
    ReservationFixture f;
    f.library.reserve(BEN, 5);
    f.library.return_book(AMIRA, 5, TODAY);
    CHECK_THROWS(f.library.borrow(CHLOE, 5, TODAY), BookNotAvailableError);
}

TEST_CASE("first in queue can borrow held copy")
{
    ReservationFixture f;
    f.library.reserve(BEN, 5);
    f.library.return_book(AMIRA, 5, TODAY);
    f.library.borrow(BEN, 5, TODAY);
    CHECK_EQ(f.library.book(5).available(), 0);
    CHECK(f.library.held_for(5).empty());
}

TEST_CASE("queue is first come first served")
{
    ReservationFixture f;
    f.library.reserve(CHLOE, 5);
    f.library.reserve(BEN, 5);
    f.library.return_book(AMIRA, 5, TODAY);
    CHECK(f.library.held_for(5) == std::vector<int>{CHLOE});
    CHECK(f.library.queue_for(5) == std::vector<int>{BEN});
}

// ------------------------------------------------------------
// Stretch: the File Loader, and a Table of Cases
// ------------------------------------------------------------

// Walks up from this source file to the first folder that holds
// data/books.csv. __FILE__ is the path the compiler was given,
// which CMake makes absolute.
std::filesystem::path find_data_dir()
{
    for (auto folder = std::filesystem::path{__FILE__}.parent_path(); folder.has_relative_path();
         folder = folder.parent_path()) {
        if (std::filesystem::exists(folder / "data" / "books.csv")) return folder / "data";
    }
    return "tasks/data";   // run from the repository root
}

TEST_CASE("corrupt csv loads good lines only")
{
    LoadResult result;
    load_books_csv(find_data_dir() / "books_corrupt.csv", result);
    std::vector<int> ids;
    for (const auto& book : result.books) ids.push_back(book->id());
    CHECK(ids == (std::vector<int>{1, 2, 4, 6, 10}));
    CHECK_EQ(result.skipped, 5);
}

// A table-driven test: one row per case. GoogleTest does this with
// TEST_P and INSTANTIATE_TEST_SUITE_P, Catch2 with GENERATE. Here a
// loop is enough: CHECK_EQ prints the values that did not match.
TEST_CASE("fines table")
{
    struct Case {
        int member_id;
        int days_late;
        int pence;
    };
    const std::array<Case, 6> cases{{
        {AMIRA, 0, 0}, {AMIRA, 1, 25}, {AMIRA, 20, 500}, {BEN, 1, 10}, {BEN, 30, 300}, {CHLOE, 365, 0},
    }};
    Library library = fixture();
    for (const Case& c : cases) {
        CHECK_EQ(library.member(c.member_id).policy().fine_for(c.days_late), c.pence);
    }
}

int main(int argc, char* argv[])
{
    std::cout << "Library rules\n";
    return mini_test::run_all(argc, argv);   // the number of failures: 0 means every test passed
}
