// ============================================================
// db.cpp: Definitions for db.h
// ============================================================

#include "db.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <utility>

// ------------------------------------------------------------
// Database
// ------------------------------------------------------------

Database::Database(const std::filesystem::path& path)
{
    // sqlite3_open takes a UTF-8 file name
    const std::string name = path.generic_string();
    if (sqlite3_open(name.c_str(), &db_) != SQLITE_OK) {
        const std::string message = db_ != nullptr ? sqlite3_errmsg(db_) : "out of memory";
        sqlite3_close(db_);   // a handle is returned even on failure
        throw DbError("cannot open " + name + ": " + message);
    }
}

Database::~Database()
{
    sqlite3_close(db_);
}

void Database::exec(const std::string& sql)
{
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        const std::string message = error != nullptr ? error : "unknown error";
        sqlite3_free(error);   // SQLite allocated it, SQLite frees it
        throw DbError(message);
    }
}

// ------------------------------------------------------------
// Statement
// ------------------------------------------------------------

Statement::Statement(Database& db, std::string_view sql) : db_{db.handle()}
{
    if (sqlite3_prepare_v2(db_, sql.data(), static_cast<int>(sql.size()), &stmt_, nullptr) != SQLITE_OK) {
        throw DbError(std::string("cannot prepare SQL: ") + sqlite3_errmsg(db_));
    }
}

Statement::~Statement()
{
    sqlite3_finalize(stmt_);
}

Statement& Statement::bind(int index, int value)
{
    sqlite3_bind_int(stmt_, index, value);
    return *this;
}

// SQLITE_TRANSIENT tells SQLite to take its own copy of the text,
// so the std::string may go away before step() runs.
Statement& Statement::bind(int index, const std::string& value)
{
    sqlite3_bind_text(stmt_, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
    return *this;
}

Statement& Statement::bind_null(int index)
{
    sqlite3_bind_null(stmt_, index);
    return *this;
}

bool Statement::step()
{
    const int result = sqlite3_step(stmt_);
    if (result == SQLITE_ROW) return true;
    if (result == SQLITE_DONE) return false;
    throw DbError(sqlite3_errmsg(db_));
}

void Statement::reset()
{
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
}

int Statement::column_int(int index) const
{
    return sqlite3_column_int(stmt_, index);
}

std::string Statement::column_text(int index) const
{
    // sqlite3_column_text returns unsigned char*, and nullptr for NULL
    const unsigned char* text = sqlite3_column_text(stmt_, index);
    return text == nullptr ? std::string{} : std::string(reinterpret_cast<const char*>(text));
}

bool Statement::column_is_null(int index) const
{
    return sqlite3_column_type(stmt_, index) == SQLITE_NULL;
}

// ------------------------------------------------------------
// Transaction
// ------------------------------------------------------------

Transaction::Transaction(Database& db) : db_{db}
{
    db_.exec("BEGIN");
}

Transaction::~Transaction()
{
    if (!done_) {
        // A destructor must not throw, so the result is ignored
        sqlite3_exec(db_.handle(), "ROLLBACK", nullptr, nullptr, nullptr);
    }
}

void Transaction::commit()
{
    db_.exec("COMMIT");
    done_ = true;
}

// ------------------------------------------------------------
// LibraryDb: Setting Up
// ------------------------------------------------------------

LibraryDb::LibraryDb(const std::filesystem::path& path) : db_{path}
{
    // SQLite ignores FOREIGN KEY rules unless this is switched on
    db_.exec("PRAGMA foreign_keys = ON");
}

void LibraryDb::create(const std::filesystem::path& schema)
{
    std::ifstream file{schema, std::ios::binary};
    if (!file) throw DbError("cannot read " + schema.generic_string());
    std::ostringstream text;
    text << file.rdbuf();
    db_.exec(text.str());   // sqlite3_exec runs every statement in the file
}

void LibraryDb::import_rows(const LoadResult& rows)
{
    Transaction transaction{db_};

    // One prepared statement, run once per row: SQLite parses the
    // SQL once, and every value goes in through a placeholder
    Statement book{db_, "INSERT INTO books (id, title, author, year, genre, copies, format, length) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"};
    for (const auto& b : rows.books) {
        book.bind(1, b->id()).bind(2, b->title()).bind(3, b->author()).bind(4, b->year());
        book.bind(5, b->genre()).bind(6, b->copies()).bind(7, std::string(b->format_name())).bind(8, b->length());
        book.step();
        book.reset();
    }

    Statement member{db_, "INSERT INTO members (id, name, email, type, joined) VALUES (?, ?, ?, ?, ?)"};
    for (const Member& m : rows.members) {
        member.bind(1, m.id()).bind(2, m.name()).bind(3, m.email()).bind(4, m.type()).bind(5, m.joined());
        member.step();
        member.reset();
    }

    Statement loan{db_, "INSERT INTO loans (id, book_id, member_id, borrowed, returned) VALUES (?, ?, ?, ?, ?)"};
    for (const Loan& l : rows.loans) {
        loan.bind(1, l.id).bind(2, l.book_id).bind(3, l.member_id).bind(4, to_iso(l.borrowed));
        if (l.returned) {
            loan.bind(5, to_iso(*l.returned));
        } else {
            loan.bind_null(5);
        }
        loan.step();
        loan.reset();
    }

    transaction.commit();   // without this line, the destructor rolls everything back
}

int LibraryDb::count(std::string_view table)
{
    // A table name cannot be a ? placeholder, so it is checked
    // against a fixed list instead of being trusted
    if (table != "books" && table != "members" && table != "loans") {
        throw DbError("unknown table " + std::string(table));
    }
    Statement query{db_, "SELECT COUNT(*) FROM " + std::string(table)};
    query.step();
    return query.column_int(0);
}

// ------------------------------------------------------------
// LibraryDb: Questions Answered in SQL
// ------------------------------------------------------------

std::vector<GenreCount> LibraryDb::genre_counts()
{
    Statement query{db_, "SELECT genre, COUNT(*) FROM books GROUP BY genre ORDER BY genre"};
    std::vector<GenreCount> rows;
    while (query.step()) {
        rows.push_back({query.column_text(0), query.column_int(1)});
    }
    return rows;
}

std::vector<ShelfRow> LibraryDb::unavailable_books()
{
    // LEFT JOIN keeps books with no open loans (COUNT(l.id) is 0 for them)
    Statement query{db_, R"(SELECT b.id, b.title, b.copies - COUNT(l.id) AS available, b.copies
                            FROM books b
                            LEFT JOIN loans l ON l.book_id = b.id AND l.returned IS NULL
                            GROUP BY b.id, b.title, b.copies
                            HAVING b.copies - COUNT(l.id) = 0
                            ORDER BY b.id)"};
    std::vector<ShelfRow> rows;
    while (query.step()) {
        rows.push_back({query.column_int(0), query.column_text(1), query.column_int(2), query.column_int(3)});
    }
    return rows;
}

std::vector<TitleRow> LibraryDb::search_titles(std::string_view text)
{
    // The % wildcards go into the value, never into the SQL
    std::string pattern = "%";
    for (char c : text) {
        pattern += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    pattern += "%";

    Statement query{db_, "SELECT id, title, year FROM books WHERE LOWER(title) LIKE ? ORDER BY id"};
    query.bind(1, pattern);
    std::vector<TitleRow> rows;
    while (query.step()) {
        rows.push_back({query.column_int(0), query.column_text(1), query.column_int(2)});
    }
    return rows;
}

std::vector<MemberLoans> LibraryDb::open_loans_per_member()
{
    Statement query{db_, R"(SELECT m.id, m.name, COUNT(l.id)
                            FROM members m
                            LEFT JOIN loans l ON l.member_id = m.id AND l.returned IS NULL
                            GROUP BY m.id, m.name
                            ORDER BY m.id)"};
    std::vector<MemberLoans> rows;
    while (query.step()) {
        rows.push_back({query.column_int(0), query.column_text(1), query.column_int(2)});
    }
    return rows;
}

// ------------------------------------------------------------
// LibraryDb: Loading the Model and Saving Changes
// ------------------------------------------------------------

Library LibraryDb::load_library()
{
    Library library;

    Statement books{db_, "SELECT id, title, author, year, genre, copies, format, length FROM books ORDER BY id"};
    while (books.step()) {
        std::vector<std::string> fields;
        for (int column = 0; column < 8; ++column) {
            fields.push_back(books.column_text(column));   // SQLite turns numbers into text for us
        }
        library.add_book(make_book(fields));
    }

    Statement members{db_, "SELECT id, name, email, type, joined FROM members ORDER BY id"};
    while (members.step()) {
        library.add_member(Member{members.column_int(0), members.column_text(1), members.column_text(2),
                                  members.column_text(3), members.column_text(4)});
    }

    Statement loans{db_, "SELECT id, book_id, member_id, borrowed, returned FROM loans ORDER BY id"};
    while (loans.step()) {
        Loan loan;
        loan.id = loans.column_int(0);
        loan.book_id = loans.column_int(1);
        loan.member_id = loans.column_int(2);
        loan.borrowed = parse_iso_date(loans.column_text(3)).value();   // value() throws if the text is bad
        if (!loans.column_is_null(4)) {
            loan.returned = parse_iso_date(loans.column_text(4)).value();
        }
        library.add_loan(loan);
    }
    return library;
}

void LibraryDb::insert_loan(const Loan& loan)
{
    Statement insert{db_, "INSERT INTO loans (id, book_id, member_id, borrowed, returned) VALUES (?, ?, ?, ?, NULL)"};
    insert.bind(1, loan.id).bind(2, loan.book_id).bind(3, loan.member_id).bind(4, to_iso(loan.borrowed));
    insert.step();
}

void LibraryDb::mark_returned(const Loan& loan)
{
    if (!loan.returned) throw DbError("loan has not been returned");
    Statement update{db_, "UPDATE loans SET returned = ? WHERE id = ?"};
    update.bind(1, to_iso(*loan.returned)).bind(2, loan.id);
    update.step();
}

// ------------------------------------------------------------
// Stretch
// ------------------------------------------------------------

// Insert the loan first, then check the stock in SQL. If the check
// fails, return without commit(): the Transaction destructor rolls
// back and the inserted row disappears.
std::optional<int> LibraryDb::borrow_in_transaction(int member_id, int book_id, Date today)
{
    Transaction transaction{db_};

    // Each SELECT is read, then reset(), so no statement is still
    // half way through when the transaction ends
    Statement next_id{db_, "SELECT COALESCE(MAX(id), 0) + 1 FROM loans"};
    next_id.step();
    const int new_id = next_id.column_int(0);
    next_id.reset();

    Statement insert{db_, "INSERT INTO loans (id, book_id, member_id, borrowed, returned) VALUES (?, ?, ?, ?, NULL)"};
    insert.bind(1, new_id).bind(2, book_id).bind(3, member_id).bind(4, to_iso(today));
    insert.step();

    Statement out{db_, "SELECT COUNT(*) FROM loans WHERE book_id = ? AND returned IS NULL"};
    out.bind(1, book_id);
    out.step();
    const int open_loans = out.column_int(0);
    out.reset();

    Statement copies{db_, "SELECT copies FROM books WHERE id = ?"};
    copies.bind(1, book_id);
    copies.step();
    const int stock = copies.column_int(0);
    copies.reset();

    if (open_loans > stock) {
        return std::nullopt;   // no commit: rolled back
    }
    transaction.commit();
    return new_id;
}

// An index on loans(member_id) lets SQLite find one member's loans
// without reading the whole loans table. It helps every query that
// joins or filters on member_id: the open loans per member query
// above, and "loans MEMBER_ID" in task 07. With 13 rows nobody
// will notice; with 13 million, everyone would.
void LibraryDb::add_member_index()
{
    db_.exec("CREATE INDEX IF NOT EXISTS loans_by_member ON loans (member_id)");
}
