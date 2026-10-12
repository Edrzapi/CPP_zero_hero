// ============================================================
// Task 07: Errors Done Properly (solution, Core + Stretch)
// ============================================================
// Session 7: exceptions (see exceptions/). Builds on task 06;
// the model is now in library.h / library.cpp.
// Reads commands, one per line, and never crashes:
//   borrow MEMBER_ID BOOK_ID | return MEMBER_ID BOOK_ID
//   loans MEMBER_ID          | quit
//
// Build and run:
//   cmake --build build --target task07_solution
//   ./build/bin/task07_solution < tasks/data/input_07.txt
//   ./build/bin/task07_solution --stretch < tasks/data/input_07.txt
//   ./build/bin/task07_solution --stretch --debug < tasks/data/input_07.txt
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 07 -- build/bin/task07_solution
// ============================================================

#include "library.h"

#include <array>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

constexpr std::array<std::string_view, 21> BOOK_LINES{
    "id,title,author,year,genre,copies,format,length",
    "1,Pride and Prejudice,Jane Austen,1813,Classic,3,print,432",
    "2,Nineteen Eighty-Four,George Orwell,1949,Dystopian,2,print,328",
    "3,The Hobbit,J. R. R. Tolkien,1937,Fantasy,0,print,310",
    "4,Dune,Frank Herbert,1965,Science Fiction,4,print,412",
    "5,Robinson Crusoe,Daniel Defoe,1719,Adventure,1,print,320",
    "6,The Time Machine,H. G. Wells,1895,Science Fiction,2,ebook,118",
    "7,Animal Farm,George Orwell,1945,Dystopian,5,print,112",
    "8,Emma,Jane Austen,1815,Classic,1,audio,1150",
    "9,The Left Hand of Darkness,Ursula K. Le Guin,1969,Science Fiction,0,print,304",
    "10,Murder on the Orient Express,Agatha Christie,1934,Crime,2,print,256",
    "11,The Murder of Roger Ackroyd,Agatha Christie,1926,Crime,1,ebook,312",
    "12,A Wizard of Earthsea,Ursula K. Le Guin,1968,Fantasy,2,print,183",
    "13,Brave New World,Aldous Huxley,1932,Dystopian,2,audio,480",
    "14,The Martian,Andy Weir,2011,Science Fiction,3,print,369",
    "15,Small Island,Andrea Levy,2004,Historical,1,print,441",
    "16,Wolf Hall,Hilary Mantel,2009,Historical,2,audio,1440",
    "17,The Remains of the Day,Kazuo Ishiguro,1989,Literary,2,print,258",
    "18,Never Let Me Go,Kazuo Ishiguro,2005,Literary,1,ebook,288",
    "19,Frankenstein,Mary Shelley,1818,Gothic,2,print,280",
    "20,The War of the Worlds,H. G. Wells,1898,Science Fiction,3,audio,410",
};

constexpr std::array<std::string_view, 10> MEMBER_LINES{
    "id,name,email,type,joined",
    "101,Amira Khan,amira.khan@example.com,standard,2023-01-15",
    "102,Ben Taylor,ben.taylor@example.com,student,2024-09-02",
    "103,Chloe Evans,chloe.evans@example.com,staff,2019-05-20",
    "104,Dev Patel,dev.patel@example.com,student,2025-09-10",
    "105,Ella Brown,ella.brown@example.com,standard,2021-03-08",
    "106,Femi Adeyemi,femi.adeyemi@example.com,standard,2022-11-30",
    "107,Grace Wilson,grace.wilson@example.com,staff,2018-07-01",
    "108,Harry Jones,harry.jones@example.com,student,2026-01-12",
    "109,Ben Taylor,ben.taylor2@example.com,standard,2025-02-14",
};

std::vector<std::string> split(std::string_view line, char separator)
{
    std::istringstream stream{std::string{line}};
    std::vector<std::string> fields;
    std::string field;
    while (std::getline(stream, field, separator)) {
        fields.push_back(field);
    }
    return fields;
}

Library load_library()
{
    Library library;
    for (std::size_t i = 1; i < BOOK_LINES.size(); ++i) {
        library.add_book(make_book(split(BOOK_LINES[i], ',')));
    }
    for (std::size_t i = 1; i < MEMBER_LINES.size(); ++i) {
        const auto f = split(MEMBER_LINES[i], ',');
        library.add_member(Member{std::stoi(f[0]), f[1], f[2], f[3], f[4]});
    }
    return library;
}

// ------------------------------------------------------------
// Turning Text into Numbers
// ------------------------------------------------------------
// std::stoi("4abc") returns 4 without complaint, and accepts
// leading spaces. std::from_chars reports where it stopped, so we
// can insist that it used every character. It never throws: it
// returns an error code, the C-style alternative to exceptions.

int parse_whole_number(std::string_view text)
{
    int value = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{}) {
        // Turn the error code into an exception (std::system_error
        // keeps the code), so it can be chained below
        throw std::system_error(std::make_error_code(error), "from_chars('" + std::string(text) + "')");
    }
    if (end != text.data() + text.size()) {
        throw std::invalid_argument("from_chars stopped early in '" + std::string(text) + "'");
    }
    return value;
}

// "4" -> 4. Anything else becomes a CommandError with our own
// message. Stretch 2: std::throw_with_nested throws the
// CommandError with the original exception stored inside it as
// the cause (exception chaining, see exceptions/02).
int parse_id(std::string_view text, std::string_view what)
{
    try {
        return parse_whole_number(text);
    } catch (const std::exception&) {
        std::throw_with_nested(
            CommandError(std::string(what) + " must be a whole number, got '" + std::string(text) + "'"));
    }
}

// Stretch 2: print the chained cause, if there is one
void print_cause(const std::exception& error)
{
    try {
        std::rethrow_if_nested(error);
    } catch (const std::exception& cause) {
        std::cout << "  cause: " << cause.what() << '\n';
    }
}

std::string to_lower(std::string_view text)
{
    std::string lower;
    for (char c : text) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}

std::string join(const std::vector<std::string>& items, std::string_view separator)
{
    std::string text;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) text += separator;
        text += items[i];
    }
    return text;
}

// ------------------------------------------------------------
// One Command
// ------------------------------------------------------------
// Returns the line to print. Throws LibraryError (or a subclass)
// when the command cannot be done.

std::string run_command(Library& library, const std::vector<std::string>& words)
{
    const std::string command = to_lower(words[0]);
    const std::size_t args = words.size() - 1;

    if (command == "borrow" || command == "return") {
        if (args != 2) {
            throw CommandError("usage: " + command + " MEMBER_ID BOOK_ID");
        }
        const int member_id = parse_id(words[1], "member id");   // the member id first
        const int book_id = parse_id(words[2], "book id");
        if (command == "borrow") {
            library.borrow(member_id, book_id);
            const Member& who = library.member(member_id);
            return "OK: " + who.name() + " borrowed " + library.book(book_id).title() + ", due in " +
                   std::to_string(who.policy().loan_days()) + " days";
        }
        library.return_book(member_id, book_id);
        return "OK: " + library.member(member_id).name() + " returned " + library.book(book_id).title();
    }

    if (command == "loans") {
        if (args != 1) {
            throw CommandError("usage: loans MEMBER_ID");
        }
        const Member& who = library.member(parse_id(words[1], "member id"));
        if (who.loans().empty()) {
            return who.name() + " has no loans";
        }
        std::vector<std::string> titles;
        for (int book_id : who.loans()) {
            titles.push_back(library.book(book_id).title());
        }
        return who.name() + " has " + plural(titles.size(), "loan", "loans") + ": " + join(titles, ", ");
    }

    throw CommandError("unknown command '" + words[0] + "'");
}

// ------------------------------------------------------------
// "finally" in C++: a Destructor
// ------------------------------------------------------------
// C++ has no finally block. An object's destructor runs however
// the scope ends (normal exit, break, return or an exception), so
// a small RAII object does the same job (see memory/02_raii.cpp).

struct Session {
    bool show_counts = false;
    int commands = 0;
    int ok = 0;
    int errors = 0;

    ~Session()
    {
        if (show_counts) {   // Stretch 1
            std::cout << "Session: " << commands << " commands, " << ok << " OK, " << errors << " errors\n";
        }
        std::cout << "Goodbye\n";
    }
};

bool has_flag(int argc, char* argv[], std::string_view flag)
{
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == flag) return true;
    }
    return false;
}

int main(int argc, char* argv[])
{
    const bool debug = has_flag(argc, argv, "--debug");
    Library library = load_library();
    std::cout << "Library Manager: " << library.book_count() << " books, " << library.member_count()
              << " members\n";

    Session session;
    session.show_counts = has_flag(argc, argv, "--stretch");

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream stream{line};
        std::vector<std::string> words;
        std::string word;
        while (stream >> word) {   // >> splits on any run of spaces
            words.push_back(word);
        }
        if (words.empty()) {       // blank lines are skipped without a word
            continue;
        }
        std::cout << "> " << join(words, " ") << '\n';
        if (to_lower(words[0]) == "quit") {
            break;
        }

        ++session.commands;
        // Catch the specific before the general: LibraryError first,
        // then anything else derived from std::exception.
        try {
            std::cout << run_command(library, words) << '\n';
            ++session.ok;
        } catch (const LibraryError& error) {
            ++session.errors;
            std::cout << "Error: " << error.what() << '\n';
            if (debug) print_cause(error);
        } catch (const std::exception&) {   // a bug of ours: report it, keep going
            ++session.errors;
            std::cout << "Error: unexpected problem\n";
        }
    }
    // session's destructor prints the counts and "Goodbye" here
}
