// ============================================================
// Task 06: Book Types and Loan Policies (starter)
// ============================================================
// Read ../README.md first. Below is the task 05 solution (Book,
// Member, Library), working as it was. At the end are the stubs
// for this task. Turn Book into an abstract base class, add the
// three formats and the policies, then write the script in main.
//
// Build and run:
//   cmake --build build --target task06_starter
//   ./build/bin/task06_starter
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 06 -- build/bin/task06_starter
// ============================================================

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
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

// ------------------------------------------------------------
// Small Helpers
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

bool is_blank(std::string_view text)
{
    return text.find_first_not_of(" \t\r\n") == std::string_view::npos;
}

std::string plural(std::size_t count, std::string_view singular, std::string_view many)
{
    return std::to_string(count) + " " + std::string(count == 1 ? singular : many);
}

// ------------------------------------------------------------
// Book: Validated on the Way In, Knows How Many Are on Loan
// ------------------------------------------------------------
// The constructor is the only way to make a Book, and it throws
// for bad values, so every Book that exists is valid. The fields
// are private: the only way to change on_loan_ is lend() and
// take_back(), which keep 0 <= on_loan_ <= copies_.

class Book {
public:
    Book(int id, std::string title, std::string author, int year, std::string genre, int copies)
        : id_{id}, title_{std::move(title)}, author_{std::move(author)}, year_{year},
          genre_{std::move(genre)}, copies_{copies}
    {
        if (id_ <= 0) throw std::invalid_argument("book id must be positive");
        if (is_blank(title_)) throw std::invalid_argument("title must not be blank");
        if (year_ < 1450 || year_ > 2100) throw std::invalid_argument("year must be between 1450 and 2100");
        if (copies_ < 0) throw std::invalid_argument("copies must not be negative");
    }

    int id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::string& author() const { return author_; }
    int year() const { return year_; }
    const std::string& genre() const { return genre_; }
    int copies() const { return copies_; }
    int available() const { return copies_ - on_loan_; }

    bool lend()
    {
        if (available() == 0) return false;
        ++on_loan_;
        return true;
    }

    bool take_back()
    {
        if (on_loan_ == 0) return false;
        --on_loan_;
        return true;
    }

    std::string to_string() const
    {
        return "#" + std::to_string(id_) + " " + title_ + " by " + author_ + " (" + std::to_string(year_) + "), " +
               std::to_string(available()) + " of " + std::to_string(copies_) + " available";
    }

    // Stretch 1: identity is the id, whatever the other fields say
    bool operator==(const Book& other) const { return id_ == other.id_; }

    // Stretch 2: natural order is title A-Z ignoring case, then year.
    // std::tie compares the pairs element by element.
    bool operator<(const Book& other) const
    {
        const std::string mine = to_lower(title_);
        const std::string theirs = to_lower(other.title_);
        return std::tie(mine, year_) < std::tie(theirs, other.year_);
    }

private:
    int id_;
    std::string title_;
    std::string author_;
    int year_;
    std::string genre_;
    int copies_;
    int on_loan_ = 0;   // default member initialiser
};

class Member {
public:
    Member(int id, std::string name, std::string email, std::string type, std::string joined)
        : id_{id}, name_{std::move(name)}, email_{std::move(email)}, type_{std::move(type)},
          joined_{std::move(joined)}
    {
        if (id_ <= 0) throw std::invalid_argument("member id must be positive");
        if (is_blank(name_)) throw std::invalid_argument("name must not be blank");
        if (type_ != "standard" && type_ != "student" && type_ != "staff") {
            throw std::invalid_argument("member type must be standard, student or staff");
        }
    }

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& email() const { return email_; }
    const std::string& type() const { return type_; }
    const std::string& joined() const { return joined_; }
    const std::vector<int>& loans() const { return loans_; }   // read-only from outside

    void add_loan(int book_id) { loans_.push_back(book_id); }

    bool has_loan(int book_id) const
    {
        return std::find(loans_.begin(), loans_.end(), book_id) != loans_.end();
    }

    void remove_loan(int book_id)
    {
        const auto found = std::find(loans_.begin(), loans_.end(), book_id);
        if (found != loans_.end()) loans_.erase(found);
    }

    std::string to_string() const
    {
        return "#" + std::to_string(id_) + " " + name_ + " (" + type_ + "), " + plural(loans_.size(), "loan", "loans");
    }

private:
    int id_;
    std::string name_;
    std::string email_;
    std::string type_;
    std::string joined_;
    std::vector<int> loans_;   // book ids on loan
};

// ------------------------------------------------------------
// Library: Owns the Books and Members, and the Borrowing Rules
// ------------------------------------------------------------
// "Not found" is a null pointer: find_by_id returns a pointer to
// the book inside the map, or nullptr. The pointer does not own
// anything; the Library does. (std::optional<Book&> is not
// allowed in C++20, and std::optional<Book> would be a copy.)

class Library {
public:
    void add_book(Book book) { books_.emplace(book.id(), std::move(book)); }
    void add_member(Member member) { members_.emplace(member.id(), std::move(member)); }

    std::size_t book_count() const { return books_.size(); }
    std::size_t member_count() const { return members_.size(); }

    const Book* find_by_id(int book_id) const
    {
        const auto found = books_.find(book_id);
        return found == books_.end() ? nullptr : &found->second;
    }

    const Member* find_member(int member_id) const
    {
        const auto found = members_.find(member_id);
        return found == members_.end() ? nullptr : &found->second;
    }

    // Titles containing text, ignoring case. std::map keeps the
    // books sorted by id, so the result is in id order too.
    std::vector<const Book*> find_by_title(std::string_view text) const
    {
        const std::string wanted = to_lower(text);
        std::vector<const Book*> found;
        for (const auto& [id, book] : books_) {
            if (to_lower(book.title()).find(wanted) != std::string::npos) {
                found.push_back(&book);
            }
        }
        return found;
    }

    bool borrow(int member_id, int book_id)
    {
        Member* member = member_for_change(member_id);
        Book* book = book_for_change(book_id);
        if (member == nullptr || book == nullptr) return false;
        if (!book->lend()) return false;
        member->add_loan(book_id);
        return true;
    }

    bool return_book(int member_id, int book_id)
    {
        Member* member = member_for_change(member_id);
        Book* book = book_for_change(book_id);
        if (member == nullptr || book == nullptr || !member->has_loan(book_id)) return false;
        member->remove_loan(book_id);
        book->take_back();
        return true;
    }

    // Stretch 3: books with at least one copy on the shelf
    std::vector<const Book*> available_books() const
    {
        std::vector<const Book*> found;
        for (const auto& [id, book] : books_) {
            if (book.available() > 0) found.push_back(&book);
        }
        return found;
    }

    std::vector<const Book*> all_books() const
    {
        std::vector<const Book*> all;
        for (const auto& [id, book] : books_) all.push_back(&book);
        return all;
    }

private:
    // Private, non-const versions: only the Library changes its books
    Book* book_for_change(int book_id)
    {
        const auto found = books_.find(book_id);
        return found == books_.end() ? nullptr : &found->second;
    }

    Member* member_for_change(int member_id)
    {
        const auto found = members_.find(member_id);
        return found == members_.end() ? nullptr : &found->second;
    }

    std::map<int, Book> books_;
    std::map<int, Member> members_;
};

Library load_library()
{
    Library library;
    for (std::size_t i = 1; i < BOOK_LINES.size(); ++i) {
        const auto f = split_csv(BOOK_LINES[i]);
        library.add_book(Book{std::stoi(f[0]), f[1], f[2], std::stoi(f[3]), f[4], std::stoi(f[5])});
    }
    for (std::size_t i = 1; i < MEMBER_LINES.size(); ++i) {
        const auto f = split_csv(MEMBER_LINES[i]);
        library.add_member(Member{std::stoi(f[0]), f[1], f[2], f[3], f[4]});
    }
    return library;
}

// ============================================================
// New in Task 06 (stubs)
// ============================================================

// Prints a placeholder so the program runs before it is finished.
void not_implemented(std::string_view what)
{
    std::cout << "not implemented: " << what << '\n';
}

// TODO: whole pence to "£P.PP" with / and % by 100.
std::string format_money([[maybe_unused]] int pence)
{
    return "not implemented";
}

// TODO Core 1 and 2: make Book abstract (pure virtual format_name()
// and length_text(), a virtual destructor, a length field and
// describe()), then derive PrintBook, EBook and AudioBook.

// TODO Core 3: std::unique_ptr<Book> make_book(fields) that reads
// the format column and builds the right type.

// TODO Core 4: an abstract LoanPolicy (loan_days, fine_per_day,
// max_loans, fine_for) and StandardPolicy, StudentPolicy,
// StaffPolicy.

// TODO Core 5: give each Member the policy for its type, and make
// Library::borrow refuse a member who already has max_loans().
// Library will need std::map<int, std::unique_ptr<Book>>.

int main()
{
    Library library = load_library();   // the task 05 version still works
    static_cast<void>(library);

    // TODO Core 6: the script in the README's Output format section
    not_implemented("Formats");
    not_implemented("Policies");
    not_implemented("Fine for 3 days late");
    not_implemented("Limits");
}
