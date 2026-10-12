// ============================================================
// library.h: the Model, Now with Loans and Dates
// ============================================================
// What changed from task 08:
//   * a Loan records who borrowed what, when, and when it came
//     back
//   * borrow() and return_book() take the date as a parameter,
//     so nothing here ever reads the real clock (tests stay
//     stable whatever day they run)
//   * due dates, days late and fines (in whole pence)
// ============================================================

#pragma once

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// ------------------------------------------------------------
// Small Helpers
// ------------------------------------------------------------

std::string plural(std::size_t count, std::string_view singular, std::string_view many);
std::string format_money(int pence);   // 125 -> "£1.25"
bool is_blank(std::string_view text);

// ------------------------------------------------------------
// Dates
// ------------------------------------------------------------
// std::chrono::sys_days is a calendar day with no time of day
// and no time zone: exactly what a library loan needs. Adding
// std::chrono::days{14} moves across month and year ends (and
// leap days) correctly, and subtracting two dates gives a whole
// number of days. See datetime/02_calendar.cpp.

using Date = std::chrono::sys_days;

// "2026-10-12" -> the date, or std::nullopt when the text is not
// a real date in exactly that form
std::optional<Date> parse_iso_date(std::string_view text);
std::string to_iso(Date day);   // always YYYY-MM-DD

// ------------------------------------------------------------
// Our Own Exceptions: One Base Class, One Subclass per Problem
// ------------------------------------------------------------
// Catch LibraryError to handle every library problem in one
// place; catch a subclass when one problem needs special care.
// std::runtime_error stores the message and what() returns it.

class LibraryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;   // inherit the (message) constructor
};

class UnknownBookError : public LibraryError {
public:
    explicit UnknownBookError(int book_id) : LibraryError("no book with id " + std::to_string(book_id)) {}
};

class UnknownMemberError : public LibraryError {
public:
    explicit UnknownMemberError(int member_id) : LibraryError("no member with id " + std::to_string(member_id)) {}
};

class BookNotAvailableError : public LibraryError {
public:
    explicit BookNotAvailableError(const std::string& title) : LibraryError(title + " has no copies available") {}
};

class MemberLimitReachedError : public LibraryError {
public:
    MemberLimitReachedError(const std::string& name, int limit)
        : LibraryError(name + " already has " + plural(static_cast<std::size_t>(limit), "loan", "loans") +
                       " (limit " + std::to_string(limit) + ")")
    {
    }
};

class NotOnLoanError : public LibraryError {
public:
    NotOnLoanError(const std::string& title, const std::string& name)
        : LibraryError(title + " is not on loan to " + name)
    {
    }
};

// The user typed something we cannot understand
class CommandError : public LibraryError {
public:
    using LibraryError::LibraryError;
};

// ------------------------------------------------------------
// Loan Policies (from task 06)
// ------------------------------------------------------------

class LoanPolicy {
public:
    virtual ~LoanPolicy() = default;
    virtual std::string_view name() const = 0;
    virtual int loan_days() const = 0;
    virtual int fine_per_day() const = 0;   // pence
    virtual int max_loans() const = 0;
    int fine_for(int days_late) const;
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

// Throws std::invalid_argument for an unknown type
const LoanPolicy& policy_for(std::string_view type);

// ------------------------------------------------------------
// Books (from task 06)
// ------------------------------------------------------------

class Book {
public:
    Book(int id, std::string title, std::string author, int year, std::string genre, int copies, int length);
    virtual ~Book() = default;

    virtual std::string_view format_name() const = 0;
    virtual std::string length_text() const = 0;
    std::string describe() const;

    int id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::string& author() const { return author_; }
    int year() const { return year_; }
    const std::string& genre() const { return genre_; }
    int copies() const { return copies_; }
    int length() const { return length_; }
    int available() const { return copies_ - on_loan_; }

    void lend();        // throws BookNotAvailableError
    void take_back();

    std::string to_string() const;
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
    using Book::Book;
    std::string_view format_name() const override { return "print"; }
    std::string length_text() const override;
};

class EBook : public Book {
public:
    using Book::Book;
    std::string_view format_name() const override { return "ebook"; }
    std::string length_text() const override;
};

class AudioBook : public Book {
public:
    using Book::Book;
    std::string_view format_name() const override { return "audio"; }
    std::string length_text() const override;
};

// The factory: the format column (fields[6]) decides the type.
// Throws std::invalid_argument for a bad format or a bad value.
std::unique_ptr<Book> make_book(const std::vector<std::string>& fields);

// ------------------------------------------------------------
// Members (from task 06)
// ------------------------------------------------------------

class Member {
public:
    Member(int id, std::string name, std::string email, std::string type, std::string joined);

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& email() const { return email_; }
    const std::string& type() const { return type_; }
    const std::string& joined() const { return joined_; }
    const LoanPolicy& policy() const { return *policy_; }
    const std::vector<int>& loans() const { return loans_; }   // book ids, in the order borrowed

    bool has_loan(int book_id) const;
    void add_loan(int book_id) { loans_.push_back(book_id); }
    void remove_loan(int book_id);

    std::string to_string() const;

private:
    int id_;
    std::string name_;
    std::string email_;
    std::string type_;
    std::string joined_;
    const LoanPolicy* policy_;
    std::vector<int> loans_;
};

// ------------------------------------------------------------
// Loan: New in Task 09
// ------------------------------------------------------------

struct Loan {
    int id = 0;
    int book_id = 0;
    int member_id = 0;
    Date borrowed;
    std::optional<Date> returned;   // empty while the book is still out

    bool is_open() const { return !returned.has_value(); }
};

// ------------------------------------------------------------
// Library: the Rules
// ------------------------------------------------------------

class Library {
public:
    void add_book(std::unique_ptr<Book> book);
    void add_member(Member member);
    // A loan from the records. An open loan takes a copy off the
    // shelf; limits are not checked (history is what it is).
    void add_loan(const Loan& loan);

    std::size_t book_count() const { return books_.size(); }
    std::size_t member_count() const { return members_.size(); }
    std::size_t loan_count() const { return loans_.size(); }
    std::vector<const Book*> all_books() const;       // by id
    std::vector<const Member*> all_members() const;   // by id
    std::vector<const Loan*> all_loans() const;       // by id

    // Titles containing text, ignoring case, by id
    std::vector<const Book*> find_by_title(std::string_view text) const;

    // "Not found" as a null pointer
    const Book* find_by_id(int book_id) const;
    const Member* find_member(int member_id) const;

    // "Not found" as an exception, for code that expects the id to exist
    const Book& book(int book_id) const;        // throws UnknownBookError
    const Member& member(int member_id) const;  // throws UnknownMemberError

    // Dates and money
    Date due_date(const Loan& loan) const;
    int days_late(const Loan& loan, Date today) const;   // never negative
    int fine(const Loan& loan, Date today) const;        // pence
    std::string status(const Loan& loan, Date today) const;
    // Open loans past their due date: most days late first, then by id
    std::vector<const Loan*> overdue(Date today) const;

    // Check order (part of the brief): member, book, limit, copies.
    // The new loan's id is the highest loan id plus 1.
    const Loan& borrow(int member_id, int book_id, Date today);
    // Check order: member, book, an open loan of that book to that member
    const Loan& return_book(int member_id, int book_id, Date today);

private:
    Book& book_for_change(int book_id);
    Member& member_for_change(int member_id);

    std::map<int, std::unique_ptr<Book>> books_;
    std::map<int, Member> members_;
    std::map<int, Loan> loans_;   // a std::map never moves its elements, so references stay valid
};
