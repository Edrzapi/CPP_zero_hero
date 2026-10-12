// ============================================================
// storage.cpp: Definitions for storage.h
// ============================================================

#include "storage.h"

#include "json.h"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string_view>
#include <system_error>

namespace {

constexpr std::size_t BOOK_FIELDS = 8;
constexpr std::size_t MEMBER_FIELDS = 5;
constexpr std::string_view BOOK_HEADER = "id,title,author,year,genre,copies,format,length";

// Every line of a text file, without the line endings.
// Opened in binary mode so the bytes are the same on every
// operating system; a "\r" left by Windows line endings is
// removed by hand. The ifstream closes itself when it goes out
// of scope (RAII), even if an exception is thrown.
std::vector<std::string> read_lines(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        throw FileNotFoundError("file not found: " + path.generic_string());
    }
    std::ifstream file{path, std::ios::binary};
    if (!file) {
        throw StorageError("cannot open " + path.generic_string());
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

// Splits on commas and keeps empty fields: "a,,b," is 4 fields.
// (std::getline(stream, field, ',') would lose the last empty one.)
std::vector<std::string> split_fields(const std::string& line)
{
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t comma = line.find(',', start);
        if (comma == std::string::npos) {
            fields.push_back(line.substr(start));
            return fields;
        }
        fields.push_back(line.substr(start, comma - start));
        start = comma + 1;
    }
}

// Digits with an optional minus sign, nothing else, and small
// enough for an int. std::stoi alone would accept " 4" and "4abc".
int whole_number(const std::string& text, std::string_view name)
{
    int value = 0;
    const char* last = text.data() + text.size();
    const auto [end, error] = std::from_chars(text.data(), last, value);
    if (text.empty() || error != std::errc{} || end != last) {
        throw std::invalid_argument(std::string(name) + " must be a whole number, got '" + text + "'");
    }
    return value;
}

std::string read_text(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        throw FileNotFoundError("file not found: " + path.generic_string());
    }
    std::ifstream file{path, std::ios::binary};
    std::ostringstream text;
    text << file.rdbuf();   // the whole file in one go
    return text.str();
}

// Opens a file for writing, creating its folder first
std::ofstream open_for_writing(const std::filesystem::path& path)
{
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    // binary: "\n" stays "\n" on Windows instead of becoming "\r\n"
    std::ofstream file{path, std::ios::binary};
    if (!file) {
        throw StorageError("cannot write " + path.generic_string());
    }
    return file;
}

}  // namespace

void LoadResult::skip(int line_number, const std::string& reason)
{
    problems.push_back("line " + std::to_string(line_number) + ": " + reason);
    ++skipped;
}

// ------------------------------------------------------------
// Reading CSV
// ------------------------------------------------------------

LoadResult load_books_csv(const std::filesystem::path& path)
{
    const std::vector<std::string> lines = read_lines(path);
    LoadResult result;
    std::vector<int> seen;
    for (std::size_t i = 1; i < lines.size(); ++i) {   // lines[0] is the header
        const int line_number = static_cast<int>(i) + 1;
        if (lines[i].empty()) continue;                 // a blank line is skipped silently
        const std::vector<std::string> row = split_fields(lines[i]);
        if (row.size() != BOOK_FIELDS) {
            result.skip(line_number, "expected 8 fields, found " + std::to_string(row.size()));
            continue;
        }
        std::unique_ptr<Book> book;
        try {
            whole_number(row[0], "id");
            whole_number(row[3], "year");
            whole_number(row[5], "copies");
            whole_number(row[7], "length");
            book = make_book(row);   // checks the format, then the Book rules
        } catch (const std::invalid_argument& error) {
            result.skip(line_number, error.what());
            continue;
        }
        if (std::find(seen.begin(), seen.end(), book->id()) != seen.end()) {
            result.skip(line_number, "duplicate book id " + std::to_string(book->id()));
            continue;
        }
        seen.push_back(book->id());
        result.books.push_back(std::move(book));
    }
    return result;
}

LoadResult load_members_csv(const std::filesystem::path& path)
{
    const std::vector<std::string> lines = read_lines(path);
    LoadResult result;
    std::vector<int> seen;
    for (std::size_t i = 1; i < lines.size(); ++i) {
        const int line_number = static_cast<int>(i) + 1;
        if (lines[i].empty()) continue;
        const std::vector<std::string> row = split_fields(lines[i]);
        if (row.size() != MEMBER_FIELDS) {
            result.skip(line_number, "expected 5 fields, found " + std::to_string(row.size()));
            continue;
        }
        try {
            Member member{whole_number(row[0], "id"), row[1], row[2], row[3], row[4]};
            if (std::find(seen.begin(), seen.end(), member.id()) != seen.end()) {
                result.skip(line_number, "duplicate member id " + std::to_string(member.id()));
                continue;
            }
            seen.push_back(member.id());
            result.members.push_back(std::move(member));
        } catch (const std::invalid_argument& error) {
            result.skip(line_number, error.what());
        }
    }
    return result;
}

// ------------------------------------------------------------
// Writing CSV and JSON
// ------------------------------------------------------------

void save_books_csv(const Library& library, const std::filesystem::path& path)
{
    std::ofstream file = open_for_writing(path);
    file << BOOK_HEADER << '\n';
    for (const Book* book : library.all_books()) {
        file << book->id() << ',' << book->title() << ',' << book->author() << ',' << book->year() << ','
             << book->genre() << ',' << book->copies() << ',' << book->format_name() << ',' << book->length()
             << '\n';
    }
}

// The same layout as Python's json.dump(..., indent=2): one key
// per line, two spaces per level, keys in a fixed order.
void save_json(const Library& library, const std::filesystem::path& path)
{
    std::ofstream file = open_for_writing(path);
    file << "{\n  \"books\": [";
    const auto books = library.all_books();
    for (std::size_t i = 0; i < books.size(); ++i) {
        const Book& b = *books[i];
        file << (i == 0 ? "\n" : ",\n") << "    {\n"
             << "      \"id\": " << b.id() << ",\n"
             << "      \"title\": " << json_quote(b.title()) << ",\n"
             << "      \"author\": " << json_quote(b.author()) << ",\n"
             << "      \"year\": " << b.year() << ",\n"
             << "      \"genre\": " << json_quote(b.genre()) << ",\n"
             << "      \"copies\": " << b.copies() << ",\n"
             << "      \"format\": " << json_quote(b.format_name()) << ",\n"
             << "      \"length\": " << b.length() << "\n"
             << "    }";
    }
    file << (books.empty() ? "]" : "\n  ]") << ",\n  \"members\": [";
    const auto members = library.all_members();
    for (std::size_t i = 0; i < members.size(); ++i) {
        const Member& m = *members[i];
        file << (i == 0 ? "\n" : ",\n") << "    {\n"
             << "      \"id\": " << m.id() << ",\n"
             << "      \"name\": " << json_quote(m.name()) << ",\n"
             << "      \"email\": " << json_quote(m.email()) << ",\n"
             << "      \"type\": " << json_quote(m.type()) << ",\n"
             << "      \"joined\": " << json_quote(m.joined()) << ",\n"
             << "      \"loans\": [";
        for (std::size_t j = 0; j < m.loans().size(); ++j) {
            file << (j == 0 ? "\n" : ",\n") << "        " << m.loans()[j];
        }
        file << (m.loans().empty() ? "]" : "\n      ]") << "\n    }";
    }
    file << (members.empty() ? "]" : "\n  ]") << "\n}\n";
}

// ------------------------------------------------------------
// Reading JSON (Stretch)
// ------------------------------------------------------------

namespace {

// The value for key, or StorageError when it is missing or has
// the wrong kind
const JsonValue& field(const JsonValue& object, std::string_view key, JsonValue::Kind kind)
{
    const JsonValue* value = object.kind == JsonValue::Kind::object ? object.find(key) : nullptr;
    if (value == nullptr || value->kind != kind) {
        throw StorageError("not a library file");
    }
    return *value;
}

int whole(const JsonValue& object, std::string_view key)
{
    return static_cast<int>(field(object, key, JsonValue::Kind::integer).integer);
}

const std::string& text(const JsonValue& object, std::string_view key)
{
    return field(object, key, JsonValue::Kind::text).text;
}

}  // namespace

LoadResult load_json(const std::filesystem::path& path)
{
    JsonValue document;
    try {
        document = parse_json(read_text(path));
    } catch (const JsonError&) {
        throw StorageError("not valid JSON");
    }

    LoadResult result;
    try {
        for (const JsonValue& item : field(document, "books", JsonValue::Kind::array).items) {
            result.books.push_back(make_book({
                std::to_string(whole(item, "id")), text(item, "title"), text(item, "author"),
                std::to_string(whole(item, "year")), text(item, "genre"), std::to_string(whole(item, "copies")),
                text(item, "format"), std::to_string(whole(item, "length")),
            }));
        }
        for (const JsonValue& item : field(document, "members", JsonValue::Kind::array).items) {
            result.members.emplace_back(whole(item, "id"), text(item, "name"), text(item, "email"),
                                        text(item, "type"), text(item, "joined"));
            for (const JsonValue& loan : field(item, "loans", JsonValue::Kind::array).items) {
                if (loan.kind != JsonValue::Kind::integer) throw StorageError("not a library file");
                result.loans.emplace_back(whole(item, "id"), static_cast<int>(loan.integer));
            }
        }
    } catch (const std::invalid_argument& error) {
        throw StorageError(error.what());   // a Book or Member rule failed
    }
    return result;
}

// ------------------------------------------------------------
// Building a Library
// ------------------------------------------------------------

Library build_library(LoadResult&& result)
{
    Library library;
    for (auto& book : result.books) {
        library.add_book(std::move(book));
    }
    for (auto& member : result.members) {
        library.add_member(std::move(member));
    }
    for (const auto& [member_id, book_id] : result.loans) {
        library.borrow(member_id, book_id);
    }
    return library;
}
