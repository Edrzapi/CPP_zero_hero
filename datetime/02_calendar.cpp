// ============================================================
// Calendar Dates (C++20)
// ============================================================
// C++20 extends <chrono> with civil calendar types: year, month,
// day, weekday, year_month_day and friends. They are the
// equivalent of Java's LocalDate or C#'s DateOnly, with
// validation and date arithmetic built in, and no time zone.
//
// Dates are printed with a small helper rather than << or
// std::format, because chrono's own I/O arrived in standard
// libraries later than the calendar types (GCC 14, MSVC 19.29).
//
// Build and run:
//   cmake --build build --target datetime_02_calendar
//   ./build/bin/datetime_02_calendar
// ============================================================

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std::chrono;   // keeps the calendar code readable; local to this file

std::string to_iso(const year_month_day& date)
{
    std::ostringstream out;
    out << static_cast<int>(date.year()) << '-' << std::setfill('0')
        << std::setw(2) << static_cast<unsigned>(date.month()) << '-'
        << std::setw(2) << static_cast<unsigned>(date.day());
    return out.str();
}

const char* weekday_name(weekday wd)
{
    static const char* names[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return names[wd.c_encoding()];   // c_encoding: Sunday = 0, as in C's tm_wday
}

// ------------------------------------------------------------
// Creating Dates
// ------------------------------------------------------------
// The / operator builds dates in three orders: y/m/d, m/d/y, d/m/y.
// 2026y and 12d are literals; October is a constant.

void creating()
{
    std::cout << "=== Creating Dates ===\n";

    year_month_day launch = 2026y / October / 12d;
    year_month_day same{year{2026}, month{10}, day{12}};
    std::cout << "  launch           : " << to_iso(launch) << '\n';
    std::cout << "  same date?       : " << std::boolalpha << (launch == same) << '\n';

    // Invalid dates can be represented and then checked.
    year_month_day bad = 2026y / February / 30d;
    std::cout << "  2026-02-30 ok()  : " << bad.ok() << '\n';                 // false

    // Leap years.
    std::cout << "  2024 is leap     : " << 2024y.is_leap() << '\n';          // true
    std::cout << "  2100 is leap     : " << year{2100}.is_leap() << '\n';     // false
}

// ------------------------------------------------------------
// Weekdays and "Last Day of the Month"
// ------------------------------------------------------------

void weekdays()
{
    std::cout << "\n=== Weekdays and Month Ends ===\n";

    year_month_day date = 2026y / October / 12d;
    weekday wd{sys_days{date}};   // converting to sys_days lets us compute the weekday
    std::cout << "  2026-10-12 is a " << weekday_name(wd) << '\n';

    year_month_day_last end_of_feb = 2028y / February / last;
    std::cout << "  last day of Feb 2028: " << static_cast<unsigned>(end_of_feb.day()) << '\n';  // 29

    // "Second Tuesday of November 2026" with weekday_indexed.
    year_month_day second_tuesday{sys_days{2026y / November / Tuesday[2]}};
    std::cout << "  2nd Tuesday of Nov 2026: " << to_iso(second_tuesday) << '\n';
}

// ------------------------------------------------------------
// Date Arithmetic
// ------------------------------------------------------------
// Add DAYS by going through sys_days (a time_point counting days).
// Adding months or years to a year_month_day keeps the day number,
// which can produce invalid dates such as 31 April: check ok().

void arithmetic()
{
    std::cout << "\n=== Date Arithmetic ===\n";

    year_month_day start = 2026y / January / 31d;

    year_month_day plus_30_days{sys_days{start} + days{30}};
    std::cout << "  " << to_iso(start) << " + 30 days   = " << to_iso(plus_30_days) << '\n';

    year_month_day plus_1_month = start + months{1};
    std::cout << "  " << to_iso(start) << " + 1 month   = " << to_iso(plus_1_month)
              << " (ok: " << plus_1_month.ok() << ")\n";
    // Clamp to the end of the month if needed:
    if (!plus_1_month.ok()) {
        plus_1_month = plus_1_month.year() / plus_1_month.month() / last;
        std::cout << "  clamped to month end  = " << to_iso(plus_1_month) << '\n';
    }

    // Days between two dates.
    auto christmas = sys_days{2026y / December / 25d};
    auto today = sys_days{2026y / October / 12d};
    std::cout << "  days from 2026-10-12 to Christmas: " << (christmas - today).count() << '\n';

    // Today's date from the system clock (UTC).
    year_month_day utc_today{floor<days>(system_clock::now())};
    std::cout << "  today (UTC) valid? " << utc_today.ok() << ", year " << static_cast<int>(utc_today.year()) << '\n';
}

int main()
{
    creating();
    weekdays();
    arithmetic();
}
