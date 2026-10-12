// ============================================================
// User-Defined Literals
// ============================================================
// operator"" _suffix lets you write 5.0_km or 90_deg. Combined
// with small "strong types" this turns unit mistakes into compile
// errors. (NASA's Mars Climate Orbiter was lost in 1999 to a
// pound-seconds vs newton-seconds mix-up.)
//
// Suffixes WITHOUT a leading underscore are reserved for the
// standard library: "text"s, 10ms, 2h, "abc"sv, 1i.
//
// Build and run:
//   cmake --build build --target under_the_hood_06_user_defined_literals
//   ./build/bin/under_the_hood_06_user_defined_literals
// ============================================================

#include <chrono>
#include <complex>
#include <iostream>
#include <numbers>
#include <string>
#include <string_view>

// ------------------------------------------------------------
// The Standard Library's Literals
// ------------------------------------------------------------

void standard_literals()
{
    using namespace std::literals;   // brings in all the std literal suffixes

    auto text = "a std::string"s;
    auto view = "a std::string_view"sv;
    auto timeout = 250ms + 2s;
    auto z = 3.0 + 4.0i;             // std::complex<double>

    std::cout << "=== Standard Literals ===\n";
    std::cout << "  " << text << " | " << view << '\n';
    std::cout << "  250ms + 2s = " << timeout.count() << " ms\n";
    std::cout << "  |3 + 4i| = " << std::abs(z) << '\n';
}

// ------------------------------------------------------------
// A Strong Type for Distance
// ------------------------------------------------------------
// One class stores metres. Literal operators convert from other
// units on the way in, so mixing kilometres and miles is safe and
// passing a bare double where a Distance is expected does not
// compile (the constructor is explicit).

class Distance {
public:
    constexpr explicit Distance(long double metres) : metres_{metres} {}
    constexpr long double metres() const { return metres_; }

    constexpr Distance operator+(Distance other) const { return Distance{metres_ + other.metres_}; }

private:
    long double metres_;
};

namespace units {

// Allowed parameter types for numeric literals:
//   unsigned long long  for integer literals (5_km)
//   long double         for floating literals (5.5_km)
constexpr Distance operator""_m(long double v) { return Distance{v}; }
constexpr Distance operator""_m(unsigned long long v) { return Distance{static_cast<long double>(v)}; }
constexpr Distance operator""_km(long double v) { return Distance{v * 1000.0L}; }
constexpr Distance operator""_km(unsigned long long v) { return Distance{static_cast<long double>(v) * 1000.0L}; }
constexpr Distance operator""_mi(long double v) { return Distance{v * 1609.344L}; }

// Angles: degrees in, radians stored.
constexpr long double operator""_deg(long double degrees) { return degrees * std::numbers::pi_v<long double> / 180.0L; }
constexpr long double operator""_deg(unsigned long long degrees)
{
    return static_cast<long double>(degrees) * std::numbers::pi_v<long double> / 180.0L;
}

}  // namespace units

double travel_time_hours(Distance d, double speed_kmh)
{
    return static_cast<double>(d.metres() / 1000.0L) / speed_kmh;
}

// ------------------------------------------------------------
// String Literal Operators
// ------------------------------------------------------------
// (const char*, std::size_t) receives the characters and length.

namespace text {

std::string operator""_shout(const char* s, std::size_t length)
{
    std::string result(s, length);
    for (char& c : result) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    }
    return result + "!";
}

}  // namespace text

int main()
{
    standard_literals();

    using namespace units;
    std::cout << "\n=== Custom Unit Literals ===\n";

    constexpr Distance run = 5_km + 400_m;
    constexpr Distance marathon = 26.2_mi;
    static_assert(run.metres() == 5400.0L);   // all computed at compile time

    std::cout << "  5_km + 400_m  = " << static_cast<double>(run.metres()) << " m\n";
    std::cout << "  26.2_mi       = " << static_cast<double>(marathon.metres()) << " m\n";
    std::cout << "  marathon at 12 km/h takes " << travel_time_hours(marathon, 12.0) << " hours\n";
    std::cout << "  90_deg        = " << static_cast<double>(90_deg) << " radians\n";
    // travel_time_hours(5.0, 12.0);   // error: no implicit conversion from double to Distance

    using namespace text;
    std::cout << "\n=== String Literal Operator ===\n";
    std::cout << "  " << "careful now"_shout << '\n';
}
