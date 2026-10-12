// ============================================================
// api.cpp: Definitions for api.h
// ============================================================

#include "api.h"

#include "json.h"
#include "openlibrary.h"

#include <charconv>
#include <climits>
#include <cstddef>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

const std::string BAD_BODY = "body must be a JSON object with whole-number member_id and book_id";

Response error(int status, const std::string& message)
{
    return {status, "{\"error\": " + json_quote(message) + "}"};
}

// ------------------------------------------------------------
// URL Pieces
// ------------------------------------------------------------

// "war%20of" -> "war of". In a query string "+" also means a space.
std::string url_decode(std::string_view text)
{
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '+') {
            out += ' ';
        } else if (text[i] == '%' && i + 2 < text.size()) {
            unsigned value = 0;
            const auto [end, err] = std::from_chars(text.data() + i + 1, text.data() + i + 3, value, 16);
            if (err == std::errc{} && end == text.data() + i + 3) {
                out += static_cast<char>(value);
                i += 2;
            } else {
                out += '%';   // not a valid escape: keep it as it is
            }
        } else {
            out += text[i];
        }
    }
    return out;
}

// The decoded value of name in "a=1&title=war%20of", if present
std::optional<std::string> query_value(std::string_view query, std::string_view name)
{
    while (!query.empty()) {
        const std::size_t amp = query.find('&');
        const std::string_view pair = query.substr(0, amp);
        const std::size_t equals = pair.find('=');
        if (equals != std::string_view::npos && url_decode(pair.substr(0, equals)) == name) {
            return url_decode(pair.substr(equals + 1));
        }
        if (amp == std::string_view::npos) break;
        query.remove_prefix(amp + 1);
    }
    return std::nullopt;
}

// "/books/4/" -> {"books", "4"}
std::vector<std::string> path_parts(std::string_view path)
{
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= path.size()) {
        const std::size_t slash = path.find('/', start);
        const std::string_view part = path.substr(start, slash - start);
        if (!part.empty()) parts.emplace_back(part);
        if (slash == std::string_view::npos) break;
        start = slash + 1;
    }
    return parts;
}

// Digits only, small enough for an int
std::optional<int> parse_id(std::string_view text)
{
    int value = 0;
    const auto [end, err] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (text.empty() || text.front() == '-' || err != std::errc{} || end != text.data() + text.size()) {
        return std::nullopt;
    }
    return value;
}

// A whole JSON number that fits in an int. Strings, decimals,
// true and false are all refused.
std::optional<int> whole_field(const JsonValue& object, std::string_view key)
{
    const JsonValue* value = object.find(key);
    if (value == nullptr || value->kind != JsonValue::Kind::integer || value->integer < INT_MIN ||
        value->integer > INT_MAX) {
        return std::nullopt;
    }
    return static_cast<int>(value->integer);
}

}  // namespace

LibraryApi::LibraryApi(Library library, Date today) : library_{std::move(library)}, today_{today} {}

// ------------------------------------------------------------
// Routing
// ------------------------------------------------------------

Response LibraryApi::handle(const Request& request)
{
    const std::vector<std::string> parts = path_parts(request.path);
    if (request.method == "GET") {
        if (parts.size() == 1 && parts[0] == "books") {
            return list_books(request.query);
        }
        if (parts.size() >= 2 && parts[0] == "books") {
            const std::optional<int> id = parse_id(parts[1]);
            if (id && parts.size() == 2) return get_book(*id);
            if (id && parts.size() == 3 && parts[2] == "openlibrary") return open_library(*id);
        }
    } else if (request.method == "POST" && parts.size() == 1 && parts[0] == "loans") {
        return create_loan(request.body);
    }
    return error(404, "not found");
}

// ------------------------------------------------------------
// Endpoints
// ------------------------------------------------------------

std::string LibraryApi::book_json(const Book& book) const
{
    return "{\"id\": " + std::to_string(book.id()) + ", \"title\": " + json_quote(book.title()) +
           ", \"author\": " + json_quote(book.author()) + ", \"year\": " + std::to_string(book.year()) +
           ", \"genre\": " + json_quote(book.genre()) + ", \"format\": " + json_quote(book.format_name()) +
           ", \"copies\": " + std::to_string(book.copies()) +
           ", \"available\": " + std::to_string(library_.free_copies(book.id())) + "}";
}

Response LibraryApi::list_books(const std::string& query)
{
    const std::optional<std::string> title = query_value(query, "title");
    const auto books = title && !title->empty() ? library_.find_by_title(*title) : library_.all_books();
    std::string body = "[";
    for (std::size_t i = 0; i < books.size(); ++i) {
        body += (i == 0 ? "" : ", ") + book_json(*books[i]);
    }
    return {200, body + "]"};
}

Response LibraryApi::get_book(int book_id)
{
    const Book* book = library_.find_by_id(book_id);
    if (book == nullptr) return error(404, "no book with id " + std::to_string(book_id));
    return {200, book_json(*book)};
}

Response LibraryApi::create_loan(const std::string& body)
{
    JsonValue data;
    try {
        data = parse_json(body);
    } catch (const JsonError&) {
        return error(400, BAD_BODY);
    }
    const std::optional<int> member_id = data.kind == JsonValue::Kind::object ? whole_field(data, "member_id") : std::nullopt;
    const std::optional<int> book_id = data.kind == JsonValue::Kind::object ? whole_field(data, "book_id") : std::nullopt;
    if (!member_id || !book_id) {
        return error(400, BAD_BODY);
    }

    try {
        const Loan& loan = library_.borrow(*member_id, *book_id, today_);
        return {201, "{\"id\": " + std::to_string(loan.id) + ", \"book_id\": " + std::to_string(loan.book_id) +
                         ", \"member_id\": " + std::to_string(loan.member_id) + ", \"borrowed\": " +
                         json_quote(to_iso(loan.borrowed)) + ", \"due\": " +
                         json_quote(to_iso(library_.due_date(loan))) + "}"};
    } catch (const UnknownMemberError& e) {
        return error(404, e.what());
    } catch (const UnknownBookError& e) {
        return error(404, e.what());
    } catch (const LibraryError& e) {
        return error(409, e.what());   // a rule said no: a conflict
    }
}

Response LibraryApi::open_library(int book_id)
{
    const Book* book = library_.find_by_id(book_id);
    if (book == nullptr) return error(404, "no book with id " + std::to_string(book_id));
    std::string match;
    try {
        match = first_match(search_open_library(book->title(), 1));
    } catch (const std::exception&) {   // no network, a timeout, or an answer that is not JSON
        return error(502, "Open Library did not answer");
    }
    return {200, "{\"id\": " + std::to_string(book->id()) + ", \"title\": " + json_quote(book->title()) +
                     ", \"open_library\": " + match + "}"};
}
