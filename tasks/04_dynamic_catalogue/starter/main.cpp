// ============================================================
// Task 04: Dynamic Catalogue (starter)
// ============================================================
// Read ../README.md first. The CSV lines, the borrowing script
// and the Book record are ready; the rest prints "not
// implemented". Task 03's solution is in ../../03_functions/
// solution/ if you want its helpers.
//
// Build and run:
//   cmake --build build --target task04_starter
//   ./build/bin/task04_starter
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 04 -- build/bin/task04_starter
// ============================================================

#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Prints a placeholder so the program runs before it is finished.
void not_implemented(std::string_view what)
{
    std::cout << "not implemented: " << what << '\n';
}

// ------------------------------------------------------------
// The Data: the Lines of data/books.csv, Copied In
// ------------------------------------------------------------

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

// The borrowing script: (action, book id), applied in order
const std::vector<std::pair<std::string, int>> ACTIONS{
    {"borrow", 1}, {"borrow", 1}, {"borrow", 4}, {"return", 1},
    {"borrow", 3}, {"borrow", 7}, {"return", 9}, {"borrow", 1},
};

struct Book {
    int id;
    std::string title;
    std::string author;
    int year;
    std::string genre;
    int copies;
    std::string format;
    int length;
};

// TODO Core 1: split the line on commas (std::getline(stream,
// field, ',') on a std::istringstream) and fill in a Book.
Book parse_book([[maybe_unused]] std::string_view line)
{
    return Book{0, "not implemented", "", 0, "", 0, "", 0};
}

// TODO Core 2 and 3: a comparator for std::map and std::set that
// compares lower-cased characters one by one (A-Z ignoring case).
struct CaseInsensitiveLess {
    bool operator()(const std::string& a, const std::string& b) const
    {
        return a < b;   // not implemented: this compares bytes
    }
};

int main()
{
    // TODO Core 1: parse BOOK_LINES (not the header) into a
    // std::vector<Book> and print "Loaded N books".
    not_implemented("Core 1: load the books");

    // TODO Core 2: std::map<std::string, std::vector<std::string>,
    // CaseInsensitiveLess> from genre to titles; print the genres.
    not_implemented("Core 2: genres");

    // TODO Core 3: a std::set of authors; print the count and names.
    not_implemented("Core 3: authors");

    // TODO Core 4: run ACTIONS against a std::map<int, int> of
    // copies on loan, printing one line per action.
    not_implemented("Core 4: borrowing");

    // TODO Core 5: "Counts", the three totals, then one line per
    // book borrowed at least once, by id.
    not_implemented("Core 5: counts");
}
