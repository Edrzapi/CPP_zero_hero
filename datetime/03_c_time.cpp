// ============================================================
// The C Time API (<ctime>) and Bridging to chrono
// ============================================================
// Before C++11 every C and C++ program used time_t and struct tm.
// You will still meet them in C libraries, logs and file formats,
// and std::put_time / strftime remain the portable way to print a
// local time on older standard libraries.
//
// Build and run:
//   cmake --build build --target datetime_03_c_time
//   ./build/bin/datetime_03_c_time
// ============================================================

#include <chrono>
#include <ctime>
#include <iomanip>   // std::put_time
#include <iostream>

// ------------------------------------------------------------
// time_t and struct tm
// ------------------------------------------------------------
// time_t: seconds since 1970-01-01 UTC on all mainstream systems
//         (64-bit nowadays; 32-bit time_t overflows in 2038).
// struct tm: broken-down calendar fields. tm_year counts from
//         1900 and tm_mon from 0: two classic off-by-one traps.
//
// std::localtime and std::gmtime return a pointer to a SHARED
// static buffer: not thread-safe, and a second call overwrites the
// first result. Thread-safe variants are platform-specific:
//   POSIX:   localtime_r(&t, &result)
//   Windows: localtime_s(&result, &t)   (arguments reversed!)
// C23 standardises localtime_r. In C++20 prefer chrono.

std::tm safe_localtime(std::time_t t)
{
    std::tm result{};
#if defined(_MSC_VER)
    localtime_s(&result, &t);         // Microsoft's version
#elif defined(_WIN32)
    result = *std::localtime(&t);     // MinGW: copy out of the shared buffer at once
#else
    localtime_r(&t, &result);         // POSIX
#endif
    return result;
}

void c_api()
{
    std::cout << "=== time_t and struct tm ===\n";

    std::time_t now = std::time(nullptr);
    std::tm local = safe_localtime(now);

    std::cout << "  time_t now        : " << static_cast<long long>(now) << '\n';
    std::cout << "  tm_year + 1900    : " << local.tm_year + 1900 << '\n';
    std::cout << "  tm_mon + 1        : " << local.tm_mon + 1 << '\n';
    std::cout << "  tm_yday (0-based) : " << local.tm_yday << '\n';

    // strftime: C formatting. Always check the return value (0 = buffer too small).
    char buffer[64];
    if (std::strftime(buffer, sizeof buffer, "%Y-%m-%d %H:%M:%S", &local) > 0) {
        std::cout << "  strftime          : " << buffer << '\n';
    }

    // std::put_time: the same format codes, as an iostream manipulator.
    std::cout << "  put_time          : " << std::put_time(&local, "%A %d %B %Y") << '\n';
}

// ------------------------------------------------------------
// Building a Time from Fields: mktime
// ------------------------------------------------------------
// mktime interprets a struct tm as LOCAL time, normalises out-of-
// range fields (day 32 becomes the 1st of next month) and returns
// a time_t. tm_isdst = -1 asks it to work out daylight saving.

void make_time()
{
    std::cout << "\n=== mktime and Normalisation ===\n";

    std::tm fields{};
    fields.tm_year = 2026 - 1900;
    fields.tm_mon = 0;        // January
    fields.tm_mday = 32;      // deliberately out of range
    fields.tm_hour = 12;
    fields.tm_isdst = -1;

    std::time_t t = std::mktime(&fields);   // also rewrites "fields" in normalised form
    std::cout << "  2026-01-32 normalised to: " << std::put_time(&fields, "%Y-%m-%d") << '\n';   // 2026-02-01

    std::time_t later = t + 3 * 24 * 60 * 60;
    std::cout << "  difftime(later, t)      : " << std::difftime(later, t) << " seconds\n";
}

// ------------------------------------------------------------
// Bridging time_t and chrono
// ------------------------------------------------------------
// system_clock::to_time_t / from_time_t convert in both directions.

void bridging()
{
    std::cout << "\n=== Converting Between chrono and time_t ===\n";

    auto now = std::chrono::system_clock::now();
    std::time_t as_c = std::chrono::system_clock::to_time_t(now);
    auto back = std::chrono::system_clock::from_time_t(as_c);   // sub-second part lost

    auto lost = std::chrono::duration_cast<std::chrono::milliseconds>(now - back);
    std::cout << "  precision lost in the round trip: " << lost.count() << " ms (under 1000)\n";

    // std::clock measures PROCESSOR time used by the program, not wall time.
    std::clock_t cpu = std::clock();
    std::cout << "  CPU time so far: " << 1000.0 * static_cast<double>(cpu) / CLOCKS_PER_SEC << " ms\n";
}

int main()
{
    c_api();
    make_time();
    bridging();
}
