// ============================================================
// Task 05: Classes (starter)
// ============================================================
// Read ../README.md first. The CSV lines and the helpers from
// task 04 are ready. Book, Member and Library are outlines: fill
// in the fields, the validation and the methods, then write the
// script in main. Task 04's solution is in
// ../../04_dynamic_catalogue/solution/.
//
// Build and run:
//   cmake --build build --target task05_starter
//   ./build/bin/task05_starter
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 05 -- build/bin/task05_starter
// ============================================================

#include <array>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Prints a placeholder so the program runs before it is finished.
void not_implemented(std::string_view what)
{
    std::cout << "not implemented: " << what << '\n';
}

// [[maybe_unused]] stops Clang warning about these constants
// until your code uses them.
[[maybe_unused]] constexpr std::array<std::string_view, 21> BOOK_LINES{
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

[[maybe_unused]] constexpr std::array<std::string_view, 10> MEMBER_LINES{
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

// ------------------------------------------------------------
// Helpers (from task 04)
// ------------------------------------------------------------

std::vector<std::string> split_csv(std::string_view line)
{
    std::istringstream stream{std::string{line}};
    std::vector<std::string> fields;
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

std::string to_lower(std::string_view text)
{
    std::string lower;
    for (char c : text) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}

// ------------------------------------------------------------
// TODO Core 1 and 3: Book
// ------------------------------------------------------------
// Private fields: id, title, author, year, genre, copies and the
// count on loan. The constructor checks each rule and throws
// std::invalid_argument("book id must be positive") and so on.
// Add getters, available(), lend(), take_back() and to_string():
//   #ID TITLE by AUTHOR (YEAR), AVAILABLE of COPIES available

class Book {
public:
    int available() const { return 0; }       // not implemented
    bool lend() { return false; }             // not implemented
    bool take_back() { return false; }        // not implemented
    std::string to_string() const { return "not implemented: Book::to_string"; }

private:
};

// ------------------------------------------------------------
// TODO Core 2 and 3: Member
// ------------------------------------------------------------
// id, name, email, type, joined and the book ids on loan.
// to_string(): #ID NAME (TYPE), N loans (1 loan for one)

class Member {
public:
    std::string to_string() const { return "not implemented: Member::to_string"; }

private:
};

// ------------------------------------------------------------
// TODO Core 4: Library
// ------------------------------------------------------------
// Books and members in std::map<int, ...> keyed by id. Not found
// is a null pointer: find_by_id returns const Book* (nullptr when
// there is no such book).

class Library {
public:
    const Book* find_by_id([[maybe_unused]] int book_id) const { return nullptr; }       // not implemented
    const Member* find_member([[maybe_unused]] int member_id) const { return nullptr; }  // not implemented
    bool borrow([[maybe_unused]] int member_id, [[maybe_unused]] int book_id) { return false; }       // not implemented
    bool return_book([[maybe_unused]] int member_id, [[maybe_unused]] int book_id) { return false; }  // not implemented

private:
};

int main()
{
    // TODO Core 5: load BOOK_LINES and MEMBER_LINES into a Library,
    // then run the script in the README's Output format section.
    not_implemented("Core 5: load the library and run the script");
}
