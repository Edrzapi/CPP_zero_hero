// ============================================================
// Task 11: Test the Rules (starter)
// ============================================================
// Read ../README.md first. library and storage are the task 09
// and 10 model, unchanged. The fixture and the first test are
// written; every other test fails with "not implemented" until
// you write it. Then add reservations test first: write one
// reservation test, watch it fail, add just enough to Library to
// make it pass, tidy up, repeat.
//
// The harness is the repository's own testing/mini_test.h:
//   TEST_CASE("name") { ... }   CHECK(condition)
//   CHECK_EQ(actual, expected)  CHECK_THROWS(expression, ExceptionType)
//
// Build and run:
//   cmake --build build --target task11_starter
//   ./build/bin/task11_starter
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 11 -- build/bin/task11_starter
// ============================================================

#include "library.h"
#include "mini_test.h"
#include "storage.h"

#include <array>
#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Makes an unfinished test fail with a clear message
[[noreturn]] void not_implemented()
{
    throw std::logic_error("not implemented");
}

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

// A fresh library for every test
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

Date day(int y, unsigned m, unsigned d)
{
    return Date{std::chrono::year{y} / std::chrono::month{m} / std::chrono::day{d}};
}

const Date TODAY = day(2026, 10, 12);

// ------------------------------------------------------------
// Borrowing
// ------------------------------------------------------------

// An example to copy: arrange (the fixture), act, assert
TEST_CASE("borrow reduces available")
{
    Library library = fixture();
    library.borrow(AMIRA, 4, TODAY);
    CHECK_EQ(library.book(4).available(), 3);
}

TEST_CASE("return restores available") { not_implemented(); }
TEST_CASE("borrow unknown book raises") { not_implemented(); }
TEST_CASE("borrow unknown member raises") { not_implemented(); }
TEST_CASE("borrow with no copies raises") { not_implemented(); }
TEST_CASE("return book not on loan raises") { not_implemented(); }
TEST_CASE("standard limit is three loans") { not_implemented(); }
TEST_CASE("student limit is five loans") { not_implemented(); }

// ------------------------------------------------------------
// Dates and Fines (build a Loan directly with the dates given)
// ------------------------------------------------------------

TEST_CASE("due date standard is 14 days") { not_implemented(); }
TEST_CASE("due date student is 21 days") { not_implemented(); }
TEST_CASE("due date staff is 28 days") { not_implemented(); }
TEST_CASE("due date crosses month and year end") { not_implemented(); }
TEST_CASE("due date in leap year") { not_implemented(); }
TEST_CASE("not overdue on due date") { not_implemented(); }
TEST_CASE("fine one day late standard") { not_implemented(); }
TEST_CASE("fine ten days late student") { not_implemented(); }
TEST_CASE("staff never fined") { not_implemented(); }
TEST_CASE("late return fine uses return date") { not_implemented(); }
TEST_CASE("format money") { not_implemented(); }

// ------------------------------------------------------------
// Reservations: write these one at a time, test first
// ------------------------------------------------------------

TEST_CASE("reserve book with no free copies joins queue") { not_implemented(); }
TEST_CASE("reserve book with free copies raises") { not_implemented(); }
TEST_CASE("reserve same book twice raises") { not_implemented(); }
TEST_CASE("returned copy is held for first in queue") { not_implemented(); }
TEST_CASE("first in queue can borrow held copy") { not_implemented(); }
TEST_CASE("queue is first come first served") { not_implemented(); }

int main(int argc, char* argv[])
{
    std::cout << "Library rules\n";
    return mini_test::run_all(argc, argv);
}
