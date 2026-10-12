// ============================================================
// std::chrono: Durations, Time Points and Clocks
// ============================================================
// <chrono> puts UNITS into the type system. A function taking
// std::chrono::milliseconds cannot be called with a bare 500 by
// mistake, and seconds convert to milliseconds automatically.
//
//   duration    an amount of time: count + unit (std::ratio)
//   time_point  a point in time: a duration since a clock's epoch
//   clock       a source of time_points: system, steady, ...
//
// Build and run:
//   cmake --build build --target datetime_01_durations_and_clocks
//   ./build/bin/datetime_01_durations_and_clocks
// ============================================================

#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;   // 5s, 250ms, 2h, 30min ...

// ------------------------------------------------------------
// Durations
// ------------------------------------------------------------
// std::chrono::seconds, milliseconds, microseconds, nanoseconds,
// minutes, hours, and C++20 days, weeks, months, years.
// Conversions that cannot lose precision happen implicitly
// (seconds -> milliseconds). Lossy ones (milliseconds -> seconds)
// need duration_cast, or floor / ceil / round (C++17).

void durations()
{
    std::cout << "=== Durations ===\n";

    std::chrono::seconds timeout = 90s;
    std::chrono::milliseconds as_ms = timeout;               // implicit: exact
    std::cout << "  90s in ms       : " << as_ms.count() << '\n';                  // 90000

    auto total = 1h + 30min + 15s;                           // common type: seconds
    std::cout << "  1h + 30min + 15s: " << total.count() << " seconds\n";          // 5415

    std::chrono::milliseconds elapsed = 2750ms;
    // std::chrono::seconds s = elapsed;                     // error: would lose precision
    auto truncated = std::chrono::duration_cast<std::chrono::seconds>(elapsed);
    auto rounded = std::chrono::round<std::chrono::seconds>(elapsed);
    std::cout << "  2750ms cast     : " << truncated.count() << "s (truncates)\n"; // 2
    std::cout << "  2750ms round    : " << rounded.count() << "s\n";               // 3

    // A floating-point representation allows fractions without a cast.
    std::chrono::duration<double> fractional = elapsed;
    std::cout << "  as double secs  : " << fractional.count() << '\n';             // 2.75

    // Custom units: a "frame" at 60 per second.
    using frames = std::chrono::duration<long long, std::ratio<1, 60>>;
    std::cout << "  1s in 60Hz frames: " << frames{1s}.count() << '\n';           // 60
}

// ------------------------------------------------------------
// Clocks
// ------------------------------------------------------------
// system_clock  wall-clock time; can jump (NTP, user changes it).
//               Use for timestamps shown to people.
// steady_clock  monotonic; never goes backwards. Use for MEASURING
//               intervals and timeouts.
// high_resolution_clock is usually an alias for one of the above;
// prefer steady_clock for timing so the choice is explicit.

long long busy_work()
{
    long long sum = 0;
    for (int i = 0; i < 5'000'000; ++i) sum += i % 7;
    return sum;
}

void timing()
{
    std::cout << "\n=== Measuring Elapsed Time ===\n";

    auto start = std::chrono::steady_clock::now();
    long long result = busy_work();
    std::this_thread::sleep_for(20ms);
    auto end = std::chrono::steady_clock::now();

    auto elapsed = end - start;   // a duration in the clock's native units
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
    std::cout << "  result " << result << ", took about " << ms.count()
              << " ms (at least 20, from the sleep)\n";

    std::cout << "  steady_clock is_steady: " << std::boolalpha << std::chrono::steady_clock::is_steady << '\n';
    std::cout << "  system_clock is_steady: " << std::chrono::system_clock::is_steady << '\n';
}

// ------------------------------------------------------------
// Time Points
// ------------------------------------------------------------
// time_point + duration = time_point; time_point - time_point =
// duration; time_point + time_point is a compile error.

void time_points()
{
    std::cout << "\n=== Time Points ===\n";

    auto now = std::chrono::system_clock::now();
    auto deadline = now + 48h;
    auto remaining = std::chrono::duration_cast<std::chrono::hours>(deadline - now);
    std::cout << "  deadline - now = " << remaining.count() << " hours\n";

    // Seconds since the Unix epoch (1970-01-01 UTC). system_clock
    // uses the Unix epoch on every platform since C++20.
    auto since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    std::cout << "  seconds since 1970: " << since_epoch.count() << '\n';
}

int main()
{
    durations();
    timing();
    time_points();
}
