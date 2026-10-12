// ============================================================
// Enumerations: enum and enum class
// ============================================================
// C's plain enum leaks its names into the surrounding scope and
// converts silently to int. C++11's enum class fixes both.
//
// Build and run:
//   cmake --build build --target basics_09_enums
//   ./build/bin/basics_09_enums
// ============================================================

#include <cstdint>
#include <iostream>
#include <string_view>
#include <type_traits>

// ------------------------------------------------------------
// Plain (Unscoped) enum
// ------------------------------------------------------------
// Enumerators are visible without a prefix and convert to int.
// Two enums in one scope cannot share an enumerator name.

enum Colour { Red, Green, Blue };          // Red == 0, Green == 1, Blue == 2
// enum TrafficLight { Red, Amber, Green }; // error: Red and Green already declared

// ------------------------------------------------------------
// Scoped enum (enum class)
// ------------------------------------------------------------
// Names must be qualified (Status::Active), there is no implicit
// conversion to int, and you can choose the underlying type.

enum class Status : std::uint8_t {
    Active = 1,
    Suspended = 2,
    Closed = 3,
};

enum class Light { Red, Amber, Green };    // no clash with Colour::Red

// A function to name each value. A switch over an enum class with
// no default lets -Wswitch warn when a new enumerator is added.
std::string_view to_string(Status s)
{
    switch (s) {
    case Status::Active:    return "Active";
    case Status::Suspended: return "Suspended";
    case Status::Closed:    return "Closed";
    }
    return "Unknown";   // reached only with an out-of-range value
}

// ------------------------------------------------------------
// Flags with enum class
// ------------------------------------------------------------
// Bitwise operators are not defined for enum class, so define
// the ones you need. std::underlying_type_t finds the int type.

enum class Permission : unsigned {
    None = 0,
    Read = 1u << 0,
    Write = 1u << 1,
    Execute = 1u << 2,
};

constexpr Permission operator|(Permission a, Permission b)
{
    using U = std::underlying_type_t<Permission>;
    return static_cast<Permission>(static_cast<U>(a) | static_cast<U>(b));
}

constexpr bool has(Permission set, Permission flag)
{
    using U = std::underlying_type_t<Permission>;
    return (static_cast<U>(set) & static_cast<U>(flag)) != 0;
}

int main()
{
    std::cout << "=== Plain enum ===\n";
    Colour c = Green;
    int as_int = c;                         // implicit conversion: allowed for plain enums
    std::cout << "Green as int = " << as_int << '\n';   // 1

    std::cout << "\n=== enum class ===\n";
    Status s = Status::Suspended;
    // int bad = s;                         // error: no implicit conversion
    auto raw = static_cast<int>(s);         // explicit conversion is fine
    std::cout << to_string(s) << " has value " << raw << '\n';
    std::cout << "sizeof(Status) = " << sizeof(Status) << " (uint8_t underlying type)\n";

    Light light = Light::Red;
    std::cout << "Light::Red == Light::Amber? " << std::boolalpha
              << (light == Light::Amber) << '\n';

    // C++20: "using enum" brings the enumerators into scope locally.
    using enum Status;
    std::cout << "using enum: " << to_string(Closed) << '\n';

    std::cout << "\n=== Flags ===\n";
    Permission perms = Permission::Read | Permission::Write;
    std::cout << "can read    : " << has(perms, Permission::Read) << '\n';     // true
    std::cout << "can execute : " << has(perms, Permission::Execute) << '\n';  // false
}
