// ============================================================
// json.cpp: a Small Recursive-Descent JSON Parser
// ============================================================
// One function per kind of value. parse_value looks at the next
// character and calls the right one; arrays and objects call
// parse_value again for their contents (that is the recursion).
// ============================================================

#include "json.h"

#include <charconv>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <system_error>

namespace {

class Parser {
public:
    explicit Parser(std::string_view text) : text_{text} {}

    JsonValue parse_document()
    {
        JsonValue value = parse_value();
        skip_spaces();
        if (pos_ != text_.size()) fail("unexpected text after the value");
        return value;
    }

private:
    [[noreturn]] void fail(const std::string& what) const
    {
        throw JsonError("not valid JSON at character " + std::to_string(pos_) + ": " + what);
    }

    void skip_spaces()
    {
        while (pos_ < text_.size() &&
               (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\n' || text_[pos_] == '\r')) {
            ++pos_;
        }
    }

    char peek()
    {
        skip_spaces();
        if (pos_ >= text_.size()) fail("the text ends too soon");
        return text_[pos_];
    }

    void expect(char wanted)
    {
        if (peek() != wanted) fail(std::string("expected '") + wanted + "'");
        ++pos_;
    }

    bool take_word(std::string_view word)
    {
        if (text_.substr(pos_, word.size()) != word) return false;
        pos_ += word.size();
        return true;
    }

    JsonValue parse_value()
    {
        const char next = peek();
        JsonValue value;
        if (next == '{') return parse_object();
        if (next == '[') return parse_array();
        if (next == '"') {
            value.kind = JsonValue::Kind::text;
            value.text = parse_string();
            return value;
        }
        if (next == '-' || (next >= '0' && next <= '9')) return parse_number();
        if (take_word("true")) {
            value.kind = JsonValue::Kind::boolean;
            value.boolean = true;
            return value;
        }
        if (take_word("false")) {
            value.kind = JsonValue::Kind::boolean;
            return value;
        }
        if (take_word("null")) return value;   // Kind::null is the default
        fail("unexpected character");
    }

    JsonValue parse_object()
    {
        JsonValue object;
        object.kind = JsonValue::Kind::object;
        expect('{');
        if (peek() == '}') {
            ++pos_;
            return object;
        }
        while (true) {
            if (peek() != '"') fail("expected a key in double quotes");
            std::string key = parse_string();
            expect(':');
            object.fields.emplace_back(std::move(key), parse_value());
            if (peek() == ',') {
                ++pos_;
                continue;
            }
            expect('}');
            return object;
        }
    }

    JsonValue parse_array()
    {
        JsonValue array;
        array.kind = JsonValue::Kind::array;
        expect('[');
        if (peek() == ']') {
            ++pos_;
            return array;
        }
        while (true) {
            array.items.push_back(parse_value());
            if (peek() == ',') {
                ++pos_;
                continue;
            }
            expect(']');
            return array;
        }
    }

    // Four hex digits after \u
    unsigned read_hex4()
    {
        if (pos_ + 4 > text_.size()) fail("short \\u escape");
        unsigned code = 0;
        const auto [end, error] = std::from_chars(text_.data() + pos_, text_.data() + pos_ + 4, code, 16);
        if (error != std::errc{} || end != text_.data() + pos_ + 4) fail("bad \\u escape");
        pos_ += 4;
        return code;
    }

    // A code point as 1 to 4 UTF-8 bytes
    static void append_utf8(std::string& out, unsigned code)
    {
        if (code < 0x80) {
            out += static_cast<char>(code);
        } else if (code < 0x800) {
            out += static_cast<char>(0xC0 | (code >> 6));
            out += static_cast<char>(0x80 | (code & 0x3F));
        } else if (code < 0x10000) {
            out += static_cast<char>(0xE0 | (code >> 12));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (code & 0x3F));
        } else {
            out += static_cast<char>(0xF0 | (code >> 18));
            out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (code & 0x3F));
        }
    }

    std::string parse_string()
    {
        expect('"');
        std::string out;
        while (true) {
            if (pos_ >= text_.size()) fail("the text ends inside a string");
            const char c = text_[pos_++];
            if (c == '"') return out;
            if (static_cast<unsigned char>(c) < 0x20) fail("control character inside a string");
            if (c != '\\') {
                out += c;
                continue;
            }
            if (pos_ >= text_.size()) fail("the text ends inside a string");
            const char escape = text_[pos_++];
            switch (escape) {
            case '"': out += '"'; break;
            case '\\': out += '\\'; break;
            case '/': out += '/'; break;
            case 'b': out += '\b'; break;
            case 'f': out += '\f'; break;
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            case 't': out += '\t'; break;
            case 'u': {
                unsigned code = read_hex4();
                // Characters outside the first 65536 come as two
                // escapes, a "surrogate pair", joined back together here
                if (code >= 0xD800 && code <= 0xDBFF && take_word("\\u")) {
                    const unsigned low = read_hex4();
                    code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                }
                append_utf8(out, code);
                break;
            }
            default: fail("unknown escape");
            }
        }
    }

    // JSON numbers: -?(0|[1-9][0-9]*)(.[0-9]+)?([eE][+-]?[0-9]+)?
    JsonValue parse_number()
    {
        const std::size_t start = pos_;
        bool whole = true;
        auto digits = [this] {
            const std::size_t first = pos_;
            while (pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9') ++pos_;
            if (pos_ == first) fail("expected a digit");
        };
        if (text_[pos_] == '-') ++pos_;
        if (pos_ < text_.size() && text_[pos_] == '0') {
            ++pos_;   // a leading 0 stands alone: 012 is not JSON
        } else {
            digits();
        }
        if (pos_ < text_.size() && text_[pos_] == '.') {
            whole = false;
            ++pos_;
            digits();
        }
        if (pos_ < text_.size() && (text_[pos_] == 'e' || text_[pos_] == 'E')) {
            whole = false;
            ++pos_;
            if (pos_ < text_.size() && (text_[pos_] == '+' || text_[pos_] == '-')) ++pos_;
            digits();
        }

        const char* first = text_.data() + start;
        const char* last = text_.data() + pos_;
        JsonValue value;
        if (whole) {
            const auto [end, error] = std::from_chars(first, last, value.integer);
            if (error == std::errc{} && end == last) {
                value.kind = JsonValue::Kind::integer;
                return value;
            }
            // too big for long long: keep it as a decimal instead
        }
        // from_chars for double is C++17 but arrived late in some
        // libraries, so strtod is used here (the text is already checked)
        value.kind = JsonValue::Kind::decimal;
        value.decimal = std::strtod(std::string(first, last).c_str(), nullptr);
        return value;
    }

    std::string_view text_;
    std::size_t pos_ = 0;
};

}  // namespace

const JsonValue* JsonValue::find(std::string_view key) const
{
    for (const auto& [name, value] : fields) {
        if (name == key) return &value;
    }
    return nullptr;
}

JsonValue parse_json(std::string_view text)
{
    return Parser{text}.parse_document();
}

std::string json_quote(std::string_view text)
{
    std::string out = "\"";
    for (char c : text) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buffer[8];
                std::snprintf(buffer, sizeof buffer, "\\u%04x", static_cast<unsigned>(c));
                out += buffer;
            } else {
                out += c;   // UTF-8 bytes pass straight through
            }
        }
    }
    return out + "\"";
}
