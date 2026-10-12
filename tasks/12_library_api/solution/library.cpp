// ============================================================
// library.cpp: Definitions for library.h
// ============================================================

#include "library.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <format>
#include <system_error>
#include <utility>

// ------------------------------------------------------------
// Small Helpers
// ------------------------------------------------------------

std::string plural(std::size_t count, std::string_view singular, std::string_view many)
{
    return std::to_string(count) + " " + std::string(count == 1 ? singular : many);
}

std::string format_money(int pence)
{
    return std::format("£{}.{:02}", pence / 100, pence % 100);
}

bool is_blank(std::string_view text)
{
    return text.find_first_not_of(" \t\r\n") == std::string_view::npos;
}

namespace {

std::string to_lower(std::string_view text)
{
    std::string lower;
    for (char c : text) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}

// The digits in text as a number, or -1 if anything else is there
int digits_value(std::string_view text)
{
    int value = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || end != text.data() + text.size() || text.front() == '-') return -1;
    return value;
}

}  // namespace

// ------------------------------------------------------------
// Dates
// ------------------------------------------------------------
// C++20 has std::chrono::parse, but library support still varies,
// so the ten characters are checked by hand: four digits, '-',
// two digits, '-', two digits.

std::optional<Date> parse_iso_date(std::string_view text)
{
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') return std::nullopt;
    const int y = digits_value(text.substr(0, 4));
    const int m = digits_value(text.substr(5, 2));
    const int d = digits_value(text.substr(8, 2));
    if (y < 0 || m < 0 || d < 0) return std::nullopt;
    const std::chrono::year_month_day day{std::chrono::year{y}, std::chrono::month{static_cast<unsigned>(m)},
                                          std::chrono::day{static_cast<unsigned>(d)}};
    if (!day.ok()) return std::nullopt;   // 2026-02-30 is not a real date
    return Date{day};
}

std::string to_iso(Date day)
{
    const std::chrono::year_month_day ymd{day};
    return std::format("{:04}-{:02}-{:02}", static_cast<int>(ymd.year()), static_cast<unsigned>(ymd.month()),
                       static_cast<unsigned>(ymd.day()));
}

// ------------------------------------------------------------
// Loan Policies
// ------------------------------------------------------------

int LoanPolicy::fine_for(int days_late) const
{
    return std::max(0, days_late) * fine_per_day();
}

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
// Books
// ------------------------------------------------------------

Book::Book(int id, std::string title, std::string author, int year, std::string genre, int copies, int length)
    : id_{id}, title_{std::move(title)}, author_{std::move(author)}, year_{year}, genre_{std::move(genre)},
      copies_{copies}, length_{length}
{
    if (id_ <= 0) throw std::invalid_argument("book id must be positive");
    if (is_blank(title_)) throw std::invalid_argument("title must not be blank");
    if (year_ < 1450 || year_ > 2100) throw std::invalid_argument("year must be between 1450 and 2100");
    if (copies_ < 0) throw std::invalid_argument("copies must not be negative");
    if (length_ <= 0) throw std::invalid_argument("length must be positive");
}

std::string Book::describe() const
{
    return "#" + std::to_string(id_) + " " + title_ + ": " + std::string(format_name()) + ", " + length_text();
}

void Book::lend()
{
    if (available() == 0) {
        throw BookNotAvailableError(title_);
    }
    ++on_loan_;
}

void Book::take_back()
{
    --on_loan_;
}

std::string Book::to_string() const
{
    return "#" + std::to_string(id_) + " " + title_ + " by " + author_ + " (" + std::to_string(year_) + "), " +
           std::to_string(available()) + " of " + std::to_string(copies_) + " available";
}

std::string PrintBook::length_text() const
{
    return std::to_string(length()) + " pages";
}

std::string EBook::length_text() const
{
    return std::to_string(length()) + " pages";
}

std::string AudioBook::length_text() const
{
    return std::format("{}h {:02}m", length() / 60, length() % 60);
}

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
// Members
// ------------------------------------------------------------

Member::Member(int id, std::string name, std::string email, std::string type, std::string joined)
    : id_{id}, name_{std::move(name)}, email_{std::move(email)}, type_{std::move(type)},
      joined_{std::move(joined)}, policy_{&policy_for(type_)}
{
    if (id_ <= 0) throw std::invalid_argument("member id must be positive");
    if (is_blank(name_)) throw std::invalid_argument("name must not be blank");
}

bool Member::has_loan(int book_id) const
{
    return std::find(loans_.begin(), loans_.end(), book_id) != loans_.end();
}

void Member::remove_loan(int book_id)
{
    const auto found = std::find(loans_.begin(), loans_.end(), book_id);
    if (found != loans_.end()) loans_.erase(found);
}

std::string Member::to_string() const
{
    return "#" + std::to_string(id_) + " " + name_ + " (" + type_ + "), " + plural(loans_.size(), "loan", "loans");
}

// ------------------------------------------------------------
// Library
// ------------------------------------------------------------

void Library::add_book(std::unique_ptr<Book> book)
{
    const int id = book->id();
    books_[id] = std::move(book);
}

void Library::add_member(Member member)
{
    const int id = member.id();
    members_.insert_or_assign(id, std::move(member));
}

void Library::add_loan(const Loan& loan)
{
    if (loan.is_open()) {
        book_for_change(loan.book_id).lend();
        member_for_change(loan.member_id).add_loan(loan.book_id);
    }
    loans_.insert_or_assign(loan.id, loan);
}

std::vector<const Book*> Library::all_books() const
{
    std::vector<const Book*> all;
    for (const auto& [id, book] : books_) all.push_back(book.get());
    return all;
}

std::vector<const Member*> Library::all_members() const
{
    std::vector<const Member*> all;
    for (const auto& [id, member] : members_) all.push_back(&member);
    return all;
}

std::vector<const Loan*> Library::all_loans() const
{
    std::vector<const Loan*> all;
    for (const auto& [id, loan] : loans_) all.push_back(&loan);
    return all;
}

std::vector<const Book*> Library::find_by_title(std::string_view text) const
{
    const std::string wanted = to_lower(text);
    std::vector<const Book*> found;
    for (const auto& [id, book] : books_) {
        if (to_lower(book->title()).find(wanted) != std::string::npos) found.push_back(book.get());
    }
    return found;
}

const Book* Library::find_by_id(int book_id) const
{
    const auto found = books_.find(book_id);
    return found == books_.end() ? nullptr : found->second.get();
}

const Member* Library::find_member(int member_id) const
{
    const auto found = members_.find(member_id);
    return found == members_.end() ? nullptr : &found->second;
}

const Book& Library::book(int book_id) const
{
    const Book* found = find_by_id(book_id);
    if (found == nullptr) throw UnknownBookError(book_id);
    return *found;
}

const Member& Library::member(int member_id) const
{
    const Member* found = find_member(member_id);
    if (found == nullptr) throw UnknownMemberError(member_id);
    return *found;
}

// const_cast is safe here: the object is not const, only the
// reference the const lookup gave back. It saves writing the
// lookup twice.
Book& Library::book_for_change(int book_id)
{
    return const_cast<Book&>(book(book_id));
}

Member& Library::member_for_change(int member_id)
{
    return const_cast<Member&>(member(member_id));
}

// ------------------------------------------------------------
// Dates and Money
// ------------------------------------------------------------

Date Library::due_date(const Loan& loan) const
{
    return loan.borrowed + std::chrono::days{member(loan.member_id).policy().loan_days()};
}

// Whole days past the due date: on the return date if the book is
// back, otherwise today. Never negative.
int Library::days_late(const Loan& loan, Date today) const
{
    const Date end = loan.returned.value_or(today);
    return std::max(0, static_cast<int>((end - due_date(loan)).count()));
}

int Library::fine(const Loan& loan, Date today) const
{
    return member(loan.member_id).policy().fine_for(days_late(loan, today));
}

std::string Library::status(const Loan& loan, Date today) const
{
    const Date due = due_date(loan);
    if (!loan.is_open()) {
        const int late = days_late(loan, today);
        return late == 0 ? "returned on time" : "returned " + plural(static_cast<std::size_t>(late), "day", "days") + " late";
    }
    // sys_days compare with < and ==; subtracting gives std::chrono::days
    if (today < due) {
        return "on loan, " + plural(static_cast<std::size_t>((due - today).count()), "day", "days") + " left";
    }
    if (today == due) {
        return "due today";
    }
    return "overdue by " + plural(static_cast<std::size_t>((today - due).count()), "day", "days");
}

std::vector<const Loan*> Library::overdue(Date today) const
{
    std::vector<const Loan*> late;
    for (const auto& [id, loan] : loans_) {
        if (loan.is_open() && days_late(loan, today) > 0) late.push_back(&loan);
    }
    // Most days late first; std::stable_sort keeps the id order for a tie
    std::stable_sort(late.begin(), late.end(), [&](const Loan* a, const Loan* b) {
        return days_late(*a, today) > days_late(*b, today);
    });
    return late;
}

// ------------------------------------------------------------
// The Rules
// ------------------------------------------------------------

const Loan& Library::borrow(int member_id, int book_id, Date today)
{
    // Every check comes before any change, so a refused borrow
    // leaves the library exactly as it was.
    Member& member = member_for_change(member_id);
    Book& book = book_for_change(book_id);
    const int limit = member.policy().max_loans();
    if (static_cast<int>(member.loans().size()) >= limit) {
        throw MemberLimitReachedError(member.name(), limit);
    }
    std::vector<int>& holds = holds_[book_id];
    const auto held = std::find(holds.begin(), holds.end(), member_id);
    if (held == holds.end() && book.available() - static_cast<int>(holds.size()) <= 0) {
        throw BookNotAvailableError(book.title());   // every free copy is held for someone else
    }
    book.lend();
    if (held != holds.end()) {
        holds.erase(held);   // the copy held for this member is now on loan to them
    }
    member.add_loan(book_id);

    // std::map is sorted by key, so the last element has the highest id
    const int new_id = loans_.empty() ? 1 : loans_.rbegin()->first + 1;
    Loan loan{new_id, book_id, member_id, today, std::nullopt};
    return loans_.emplace(new_id, loan).first->second;
}

const Loan& Library::return_book(int member_id, int book_id, Date today)
{
    Member& member = member_for_change(member_id);
    Book& book = book_for_change(book_id);
    for (auto& [id, loan] : loans_) {   // by id, so the oldest open loan first
        if (loan.is_open() && loan.member_id == member_id && loan.book_id == book_id) {
            loan.returned = today;
            member.remove_loan(book_id);
            book.take_back();
            // First come, first served: the copy is held for the first in the queue
            std::deque<int>& queue = queues_[book_id];
            if (!queue.empty()) {
                holds_[book_id].push_back(queue.front());
                queue.pop_front();
            }
            return loan;
        }
    }
    throw NotOnLoanError(book.title(), member.name());
}

// ------------------------------------------------------------
// Reservations
// ------------------------------------------------------------

int Library::free_copies(int book_id) const
{
    const auto holds = holds_.find(book_id);
    const int held = holds == holds_.end() ? 0 : static_cast<int>(holds->second.size());
    return book(book_id).available() - held;
}

int Library::reserve(int member_id, int book_id)
{
    const Member& who = member(member_id);
    const Book& wanted = book(book_id);
    if (free_copies(book_id) > 0) {
        throw ReservationNotNeededError(wanted.title());
    }
    std::deque<int>& queue = queues_[book_id];
    if (std::find(queue.begin(), queue.end(), member_id) != queue.end()) {
        throw AlreadyReservedError(who.name(), wanted.title());
    }
    queue.push_back(member_id);
    return static_cast<int>(queue.size());
}

std::vector<int> Library::queue_for(int book_id) const
{
    const auto found = queues_.find(book_id);
    return found == queues_.end() ? std::vector<int>{} : std::vector<int>(found->second.begin(), found->second.end());
}

std::vector<int> Library::held_for(int book_id) const
{
    const auto found = holds_.find(book_id);
    return found == holds_.end() ? std::vector<int>{} : found->second;
}
