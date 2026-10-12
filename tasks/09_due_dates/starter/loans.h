// ============================================================
// loans.h: New in Task 09 (starter)
// ============================================================
// Everything else in this folder is the task 08 solution,
// unchanged. Fill in loans.cpp, then give Library a std::map of
// loans and the date rules (see the README):
//   due_date(loan), days_late(loan, today), fine(loan, today),
//   status(loan, today), overdue(today),
//   borrow(member_id, book_id, today) and
//   return_book(member_id, book_id, today) that record a Loan.
// Load data/loans.csv in storage.cpp (an empty "returned" field
// means the book is still out).
// ============================================================

#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

// A calendar day with no time and no time zone
using Date = std::chrono::sys_days;

// "2026-10-12" -> the date, or std::nullopt when the text is not a
// real date in exactly that form
std::optional<Date> parse_iso_date(std::string_view text);

// Always YYYY-MM-DD
std::string to_iso(Date day);

struct Loan {
    int id = 0;
    int book_id = 0;
    int member_id = 0;
    Date borrowed;
    std::optional<Date> returned;   // empty while the book is still out

    bool is_open() const { return !returned.has_value(); }
};
