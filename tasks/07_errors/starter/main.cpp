// ============================================================
// Task 07: Errors Done Properly (starter)
// ============================================================
// Read ../README.md first. Below is the task 06 solution, where
// borrow() and return_book() still return true or false. At the
// end are the stubs for this task: the exception classes and the
// command loop. Make borrow() and return_book() throw instead.
// (The solution also moves the model into library.h and
// library.cpp; you may keep it in one file.)
//
// Build and run:
//   cmake --build build --target task07_starter
//   ./build/bin/task07_starter < tasks/data/input_07.txt
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 07 -- build/bin/task07_starter
// ============================================================

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
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

bool is_blank(std::string_view text)
{
    return text.find_first_not_of(" \t\r\n") == std::string_view::npos;
}

std::string plural(std::size_t count, std::string_view singular, std::string_view many)
{
    return std::to_string(count) + " " + std::string(count == 1 ? singular : many);
}

// Whole pence to pounds: 125 -> "£1.25". Integer maths only.
// The source is compiled as UTF-8 (/utf-8), so "£" is the two
// bytes C2 A3. Never pad a string holding it with setw or {:<N}:
// the width counts bytes, not characters.
std::string format_money(int pence)
{
    return std::format("£{}.{:02}", pence / 100, pence % 100);
}

// ------------------------------------------------------------
// Loan Policies: One Interface, Three Implementations
// ------------------------------------------------------------
// C++ has no interface keyword: an abstract class with only pure
// virtual functions (= 0) plays the part. fine_for is written
// once here and inherited by every policy.

class LoanPolicy {
public:
    virtual ~LoanPolicy() = default;

    virtual std::string_view name() const = 0;
    virtual int loan_days() const = 0;
    virtual int fine_per_day() const = 0;   // pence for each day late
    virtual int max_loans() const = 0;

    int fine_for(int days_late) const { return std::max(0, days_late) * fine_per_day(); }

    std::string describe() const
    {
        const std::string fine = fine_per_day() == 0 ? "no fines" : format_money(fine_per_day()) + " a day";
        return std::format("{}: {} days, {}, up to {}", name(), loan_days(), fine,
                           plural(static_cast<std::size_t>(max_loans()), "loan", "loans"));
    }
};

class StandardPolicy : public LoanPolicy {
public:
    std::string_view name() const override { return "standard"; }
    int loan_days() const override { return 14; }
    int fine_per_day() const override { return 25; }
    int max_loans() const override { return 3; }
};

class StudentPolicy : public LoanPolicy {
public:
    std::string_view name() const override { return "student"; }
    int loan_days() const override { return 21; }
    int fine_per_day() const override { return 10; }
    int max_loans() const override { return 5; }
};

class StaffPolicy : public LoanPolicy {
public:
    std::string_view name() const override { return "staff"; }
    int loan_days() const override { return 28; }
    int fine_per_day() const override { return 0; }
    int max_loans() const override { return 10; }
};

// Stretch 1: a new policy. Nothing in Library or Member changes;
// only policy_for would need one more line to hand it out.
class GuestPolicy : public LoanPolicy {
public:
    std::string_view name() const override { return "guest"; }
    int loan_days() const override { return 7; }
    int fine_per_day() const override { return 50; }
    int max_loans() const override { return 1; }
};

// One shared object per policy, made on first use (a function
// local static). Members point at them; nobody owns or deletes
// them, they live until the program ends.
const LoanPolicy& policy_for(std::string_view type)
{
    static const StandardPolicy standard;
    static const StudentPolicy student;
    static const StaffPolicy staff;
    if (type == "standard") return standard;
    if (type == "student") return student;
    if (type == "staff") return staff;
    throw std::invalid_argument("member type must be standard, student or staff");
}

// ------------------------------------------------------------
// Books: an Abstract Base and Three Concrete Types
// ------------------------------------------------------------

class Book {
public:
    Book(int id, std::string title, std::string author, int year, std::string genre, int copies, int length)
        : id_{id}, title_{std::move(title)}, author_{std::move(author)}, year_{year},
          genre_{std::move(genre)}, copies_{copies}, length_{length}
    {
        if (id_ <= 0) throw std::invalid_argument("book id must be positive");
        if (is_blank(title_)) throw std::invalid_argument("title must not be blank");
        if (year_ < 1450 || year_ > 2100) throw std::invalid_argument("year must be between 1450 and 2100");
        if (copies_ < 0) throw std::invalid_argument("copies must not be negative");
        if (length_ <= 0) throw std::invalid_argument("length must be positive");
    }

    // A base class used through pointers needs a virtual destructor,
    // or deleting a PrintBook through a Book* skips ~PrintBook.
    virtual ~Book() = default;

    // Each type answers these in its own way (pure virtual: = 0)
    virtual std::string_view format_name() const = 0;
    virtual std::string length_text() const = 0;

    // Written once, works for every type: polymorphism
    std::string describe() const
    {
        return "#" + std::to_string(id_) + " " + title_ + ": " + std::string(format_name()) + ", " + length_text();
    }

    int id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::string& author() const { return author_; }
    int year() const { return year_; }
    const std::string& genre() const { return genre_; }
    int copies() const { return copies_; }
    int length() const { return length_; }
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

    bool operator==(const Book& other) const { return id_ == other.id_; }

private:
    int id_;
    std::string title_;
    std::string author_;
    int year_;
    std::string genre_;
    int copies_;
    int length_;
    int on_loan_ = 0;
};

class PrintBook : public Book {
public:
    using Book::Book;   // inherit the constructor
    std::string_view format_name() const override { return "print"; }
    std::string length_text() const override { return std::to_string(length()) + " pages"; }
};

class EBook : public Book {
public:
    using Book::Book;
    std::string_view format_name() const override { return "ebook"; }
    std::string length_text() const override { return std::to_string(length()) + " pages"; }
};

class AudioBook : public Book {
public:
    using Book::Book;
    std::string_view format_name() const override { return "audio"; }
    // length is in minutes: 1150 -> "19h 10m"
    std::string length_text() const override { return std::format("{}h {:02}m", length() / 60, length() % 60); }
};

// The factory: the format column decides which class to build.
// It returns ownership in a unique_ptr, so the caller cannot leak
// the book and nothing is ever sliced down to a plain Book.
std::unique_ptr<Book> make_book(const std::vector<std::string>& f)
{
    const std::string& format = f[6];
    if (format != "print" && format != "ebook" && format != "audio") {
        throw std::invalid_argument("format must be print, ebook or audio, got '" + format + "'");
    }
    const int id = std::stoi(f[0]);
    const int year = std::stoi(f[3]);
    const int copies = std::stoi(f[5]);
    const int length = std::stoi(f[7]);
    if (format == "print") return std::make_unique<PrintBook>(id, f[1], f[2], year, f[4], copies, length);
    if (format == "ebook") return std::make_unique<EBook>(id, f[1], f[2], year, f[4], copies, length);
    return std::make_unique<AudioBook>(id, f[1], f[2], year, f[4], copies, length);
}

// ------------------------------------------------------------
// Members, Each with a Policy
// ------------------------------------------------------------

class Member {
public:
    Member(int id, std::string name, std::string email, std::string type, std::string joined)
        : id_{id}, name_{std::move(name)}, email_{std::move(email)}, type_{std::move(type)},
          joined_{std::move(joined)}, policy_{&policy_for(type_)}   // throws for an unknown type
    {
        if (id_ <= 0) throw std::invalid_argument("member id must be positive");
        if (is_blank(name_)) throw std::invalid_argument("name must not be blank");
    }

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& type() const { return type_; }
    const LoanPolicy& policy() const { return *policy_; }
    const std::vector<int>& loans() const { return loans_; }

    bool can_borrow_more() const { return static_cast<int>(loans_.size()) < policy_->max_loans(); }
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
    const LoanPolicy* policy_;   // points at a shared policy, does not own it
    std::vector<int> loans_;
};

// ------------------------------------------------------------
// Library: One Owner for Every Book
// ------------------------------------------------------------
// std::map<int, std::unique_ptr<Book>>: the Library owns each
// book, there is no delete anywhere, and callers only ever see
// const Book* (look, do not own).

class Library {
public:
    void add_book(std::unique_ptr<Book> book)
    {
        const int id = book->id();
        books_.emplace(id, std::move(book));
    }

    void add_member(Member member) { members_.emplace(member.id(), std::move(member)); }

    const Book* find_by_id(int book_id) const
    {
        const auto found = books_.find(book_id);
        return found == books_.end() ? nullptr : found->second.get();
    }

    const Member* find_member(int member_id) const
    {
        const auto found = members_.find(member_id);
        return found == members_.end() ? nullptr : &found->second;
    }

    std::vector<const Book*> all_books() const
    {
        std::vector<const Book*> all;
        for (const auto& [id, book] : books_) all.push_back(book.get());
        return all;
    }

    bool borrow(int member_id, int book_id)
    {
        Member* member = member_for_change(member_id);
        Book* book = book_for_change(book_id);
        if (member == nullptr || book == nullptr) return false;
        if (!member->can_borrow_more()) return false;   // the policy decides the limit
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

private:
    Book* book_for_change(int book_id)
    {
        const auto found = books_.find(book_id);
        return found == books_.end() ? nullptr : found->second.get();
    }

    Member* member_for_change(int member_id)
    {
        const auto found = members_.find(member_id);
        return found == members_.end() ? nullptr : &found->second;
    }

    std::map<int, std::unique_ptr<Book>> books_;
    std::map<int, Member> members_;
};

Library load_library()
{
    Library library;
    for (std::size_t i = 1; i < BOOK_LINES.size(); ++i) {
        library.add_book(make_book(split_csv(BOOK_LINES[i])));
    }
    for (std::size_t i = 1; i < MEMBER_LINES.size(); ++i) {
        const auto f = split_csv(MEMBER_LINES[i]);
        library.add_member(Member{std::stoi(f[0]), f[1], f[2], f[3], f[4]});
    }
    return library;
}

// ============================================================
// New in Task 07 (stubs)
// ============================================================

// TODO Core 1: LibraryError (derived from std::runtime_error) and
// one subclass per problem, each building its own message:
//   UnknownBookError       no book with id ID
//   UnknownMemberError     no member with id ID
//   BookNotAvailableError  TITLE has no copies available
//   MemberLimitReachedError NAME already has N loans (limit N)
//   NotOnLoanError         TITLE is not on loan to NAME
//   CommandError           (the message passed in)

class LibraryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// TODO Core 3 and 5: turn the words of one line into a command and
// run it, returning the line to print. Throw CommandError for bad
// input; std::from_chars (in <charconv>) parses an id and tells
// you whether it used every character.
std::string run_command([[maybe_unused]] Library& library, [[maybe_unused]] const std::vector<std::string>& words)
{
    throw LibraryError("not implemented: run_command");
}

int main()
{
    Library library = load_library();
    static_cast<void>(library);
    std::cout << "Library Manager: 20 books, 9 members\n";

    // TODO Core 3, 4 and 6: read lines with std::getline until "quit"
    // or the end of the input. Skip blank lines, echo the others as
    // "> WORDS", call run_command and print its result, or catch
    // LibraryError and print "Error: MESSAGE" (and catch anything
    // else as "Error: unexpected problem").
    std::cout << "not implemented: the command loop\n";
    std::cout << "Goodbye\n";
}
