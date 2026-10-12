// ============================================================
// json.h: Just Enough JSON (the task 08 Stretch solution)
// ============================================================
// parse_json reads a request body into a JsonValue tree and
// throws JsonError for anything that is not JSON. json_quote
// writes text safely into the JSON you build by hand.
// ============================================================

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class JsonError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// One JSON value. kind says which of the members is in use.
// A struct may hold a std::vector of itself (allowed since C++17).
struct JsonValue {
    enum class Kind { null, boolean, integer, decimal, text, array, object };

    // Declared so Clang with libstdc++ does not check the default
    // constructor of std::pair<std::string, JsonValue> (in fields
    // below) while JsonValue is still incomplete.
    JsonValue() = default;

    Kind kind = Kind::null;
    bool boolean = false;
    long long integer = 0;   // a number written without '.', 'e' or 'E'
    double decimal = 0.0;    // any other number
    std::string text;
    std::vector<JsonValue> items;                            // array
    std::vector<std::pair<std::string, JsonValue>> fields;   // object, in file order

    // The value for key in an object, or nullptr when missing
    const JsonValue* find(std::string_view key) const;
};

// Throws JsonError when text is not one complete JSON value
JsonValue parse_json(std::string_view text);

// "text" with quotes, and backslash escapes for ", \ and control
// characters, ready to write into a JSON file
std::string json_quote(std::string_view text);
