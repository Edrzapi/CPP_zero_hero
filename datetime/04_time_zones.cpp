// ============================================================
// Time Zones (C++20, Library Support Varies)
// ============================================================
// C++20 adds the IANA time zone database to <chrono>: zoned_time
// pairs an instant with a zone such as "Europe/London", and
// handles daylight saving correctly. Like Java's ZonedDateTime.
//
// Support at the time of writing:
//   MSVC 19.29+ (VS 2019 16.10+), uses the Windows ICU data
//   GCC 14+ (libstdc++), reads the system tzdata
//   Clang with libc++: partial, not enabled by default
// This file checks the __cpp_lib_chrono feature-test macro and
// prints a note instead of failing to build where support is
// missing. The same technique works for any newer library feature.
//
// Build and run:
//   cmake --build build --target datetime_04_time_zones
//   ./build/bin/datetime_04_time_zones
// ============================================================

#include <chrono>
#include <iostream>
#include <version>   // feature-test macros for the whole library

#if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L && defined(__cpp_lib_format)
#define CPPZH_HAS_TIME_ZONES 1
#include <exception>
#include <format>
#endif

int main()
{
    std::cout << "=== Time Zones ===\n";

#ifdef CPPZH_HAS_TIME_ZONES
    try {
        using namespace std::chrono;

        // A fixed instant: 2026-10-12 09:30 UTC.
        // %Z abbreviations depend on the platform's data: Linux
        // prints EDT and JST, Windows (ICU) may print GMT-4 and GMT+9.
        sys_seconds instant = sys_days{2026y / October / 12d} + 9h + 30min;

        for (const char* zone : {"UTC", "Europe/London", "America/New_York", "Asia/Tokyo"}) {
            zoned_time local{zone, instant};
            std::cout << std::format("  {:<18} {:%Y-%m-%d %H:%M %Z}\n", zone, local);
        }

        // Daylight saving: the same wall-clock time in winter and summer.
        auto london = locate_zone("Europe/London");
        auto winter = zoned_time{london, sys_days{2026y / January / 15d} + 12h};
        auto summer = zoned_time{london, sys_days{2026y / July / 15d} + 12h};
        std::cout << std::format("  London in January at 12:00 UTC: {:%H:%M %Z}\n", winter);
        std::cout << std::format("  London in July    at 12:00 UTC: {:%H:%M %Z}\n", summer);

        // The machine's own zone.
        std::cout << "  this machine's zone: " << current_zone()->name() << '\n';
    } catch (const std::exception& e) {
        // Thrown when the tz database cannot be found at run time
        // (for example a minimal container image without tzdata).
        std::cout << "  time zone database unavailable: " << e.what() << '\n';
    }
#else
    std::cout << "  This standard library does not provide C++20 time zones yet\n"
              << "  (__cpp_lib_chrono < 201907L). The calendar types in\n"
              << "  02_calendar.cpp still work. Alternative: Howard Hinnant's\n"
              << "  date/tz library, which the standard version is based on.\n";
#endif
}
