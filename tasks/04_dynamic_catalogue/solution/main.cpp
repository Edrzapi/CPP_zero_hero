// ============================================================
// Task 04: Dynamic Catalogue (solution, Core + Stretch)
// ============================================================
// Session 4: collections (see collections/). Builds on task 03:
// the fixed shelf of 10 becomes a std::vector of all 20 books,
// plus std::map and std::set.
//
// Build and run:
//   cmake --build build --target task04_solution
//   ./build/bin/task04_solution              Core output (checked)
//   ./build/bin/task04_solution --stretch    Core, then the Stretch
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 04 -- build/bin/task04_solution
// ============================================================

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// The Data: the Lines of data/books.csv, Copied In
// ------------------------------------------------------------
// Reading files comes in task 08. Until then the program carries
// its own copy of the CSV lines, header first.

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

// The borrowing script: (action, book id), applied in order
const std::vector<std::pair<std::string, int>> ACTIONS{
    {"borrow", 1}, {"borrow", 1}, {"borrow", 4}, {"return", 1},
    {"borrow", 3}, {"borrow", 7}, {"return", 9}, {"borrow", 1},
};

// ------------------------------------------------------------
// A Book Record
// ------------------------------------------------------------
// A plain struct: data with no behaviour (classes come in 05).

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

Book parse_book(std::string_view line)
{
    std::istringstream stream{std::string{line}};
    std::vector<std::string> f;
    std::string field;
    while (std::getline(stream, field, ',')) {
        f.push_back(field);
    }
    return Book{std::stoi(f[0]), f[1], f[2], std::stoi(f[3]), f[4], std::stoi(f[5]), f[6], std::stoi(f[7])};
}

// ------------------------------------------------------------
// A-Z Ignoring Case
// ------------------------------------------------------------
// std::map and std::set keep their keys sorted with a comparator.
// The default (std::less) compares bytes, so "Zebra" < "apple".
// This one lower-cases each character first: ordinal, not
// locale-aware. Note that it also makes "Dune" and "DUNE" the
// same key, which is what A-Z ignoring case means.

struct CaseInsensitiveLess {
    bool operator()(const std::string& a, const std::string& b) const
    {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char x, char y) {
            return std::tolower(static_cast<unsigned char>(x)) < std::tolower(static_cast<unsigned char>(y));
        });
    }
};

std::string join(const std::vector<std::string>& items, std::string_view separator)
{
    std::string text;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) text += separator;
        text += items[i];
    }
    return text;
}

bool has_flag(int argc, char* argv[], std::string_view flag)
{
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == flag) return true;
    }
    return false;
}

int main(int argc, char* argv[])
{
    // --------------------------------------------------------
    // Core 1: every line into a vector (skipping the header)
    // --------------------------------------------------------
    std::vector<Book> books;
    for (std::size_t i = 1; i < BOOK_LINES.size(); ++i) {
        books.push_back(parse_book(BOOK_LINES[i]));
    }
    std::cout << "Loaded " << books.size() << " books\n";

    // --------------------------------------------------------
    // Core 2: index by genre (map of genre -> titles)
    // --------------------------------------------------------
    // operator[] creates an empty vector the first time a genre
    // is seen, so there is no "is it there yet?" check.

    std::map<std::string, std::vector<std::string>, CaseInsensitiveLess> by_genre;
    for (const Book& book : books) {
        by_genre[book.genre].push_back(book.title);
    }

    std::cout << "Genres (" << by_genre.size() << ")\n";
    for (const auto& [genre, titles] : by_genre) {   // already in A-Z order
        std::cout << genre << " (" << titles.size() << "): " << join(titles, ", ") << '\n';
    }

    // --------------------------------------------------------
    // Core 3: unique authors (a set drops the duplicates)
    // --------------------------------------------------------
    std::set<std::string, CaseInsensitiveLess> authors;
    for (const Book& book : books) {
        authors.insert(book.author);
    }
    std::cout << "Authors (" << authors.size() << " unique across " << books.size() << " books)\n";
    for (const std::string& author : authors) {
        std::cout << author << '\n';
    }

    // --------------------------------------------------------
    // Core 4 and 5: borrow and return, counted in maps by id
    // --------------------------------------------------------
    std::map<int, const Book*> catalogue;   // pointers into books: no copies
    for (const Book& book : books) {
        catalogue[book.id] = &book;
    }

    std::map<int, int> on_loan;
    std::map<int, int> borrowed_count;
    std::map<int, int> returned_count;
    int borrows = 0;
    int returns = 0;
    int refused = 0;

    std::cout << "Borrowing\n";
    for (const auto& [action, book_id] : ACTIONS) {
        const Book& book = *catalogue.at(book_id);
        int& out = on_loan[book_id];   // a reference: changing out changes the map
        if (action == "borrow") {
            if (out < book.copies) {
                ++out;
                ++borrowed_count[book_id];
                ++borrows;
                std::cout << "borrow " << book_id << ": OK, " << book.copies - out << " left\n";
            } else {
                ++refused;
                std::cout << "borrow " << book_id << ": refused, none available\n";
            }
        } else {
            if (out > 0) {
                --out;
                ++returned_count[book_id];
                ++returns;
                std::cout << "return " << book_id << ": OK, " << book.copies - out << " left\n";
            } else {
                ++refused;
                std::cout << "return " << book_id << ": refused, none on loan\n";
            }
        }
    }

    std::cout << "Counts\n";
    std::cout << "borrows: " << borrows << '\n';
    std::cout << "returns: " << returns << '\n';
    std::cout << "refused: " << refused << '\n';
    for (const auto& [book_id, count] : borrowed_count) {   // std::map: sorted by id
        // find() rather than [] so reading never adds a key
        const auto returned = returned_count.find(book_id);
        const int back = returned == returned_count.end() ? 0 : returned->second;
        std::cout << "book " << book_id << ": borrowed " << count << ", returned " << back << '\n';
    }

    // --------------------------------------------------------
    // Stretch: busy authors, books per decade, biggest genre
    // --------------------------------------------------------
    if (has_flag(argc, argv, "--stretch")) {
        std::cout << "Stretch\n";

        std::map<std::string, int, CaseInsensitiveLess> per_author;
        for (const Book& book : books) {
            ++per_author[book.author];
        }
        std::vector<std::string> busy;
        for (const auto& [author, count] : per_author) {
            if (count > 1) busy.push_back(author);
        }
        std::cout << "Authors with more than one book: " << join(busy, ", ") << '\n';

        std::map<int, int> per_decade;
        for (const Book& book : books) {
            ++per_decade[book.year / 10 * 10];   // integer division: 1813 -> 1810
        }
        for (const auto& [decade, count] : per_decade) {
            std::cout << decade << "s: " << count << '\n';
        }

        // Most books wins; a tie keeps the genre first in A-Z order
        std::string biggest;
        std::size_t most = 0;
        for (const auto& [genre, titles] : by_genre) {
            if (titles.size() > most) {
                biggest = genre;
                most = titles.size();
            }
        }
        std::cout << "Biggest genre: " << biggest << " (" << most << " books)\n";
    }
}
