// ============================================================
// loans.cpp: Definitions for loans.h (starter)
// ============================================================

#include "loans.h"

// TODO: check the shape (4 digits, '-', 2 digits, '-', 2 digits),
// read the numbers with std::from_chars, build a
// std::chrono::year_month_day and check ok() before returning it.
std::optional<Date> parse_iso_date([[maybe_unused]] std::string_view text)
{
    return std::nullopt;   // not implemented
}

// TODO: std::chrono::year_month_day{day} gives the year, month and
// day; std::format("{:04}-{:02}-{:02}", ...) puts them together.
std::string to_iso([[maybe_unused]] Date day)
{
    return "not implemented";
}
