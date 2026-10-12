// ============================================================
// db.h: the Library in SQLite
// ============================================================
// SQLite is a C library. No extra download is needed: the
// operating system already has it.
//   Windows 10 and 11: winsqlite3.dll is part of Windows; the
//     header <winsqlite/winsqlite3.h> and winsqlite3.lib come
//     with the Windows SDK (installed with Visual Studio)
//   macOS: the system libsqlite3 (<sqlite3.h>, -lsqlite3)
//   Linux: the libsqlite3-dev package
// All three give the same C API. tasks/CMakeLists.txt links the
// right one.
//
// A C API hands out raw handles (sqlite3*, sqlite3_stmt*) that
// must be closed by hand. Database and Statement below wrap them
// in RAII classes, so a handle is always closed, even when an
// exception is thrown (the same idea as c_interop/02_c_library).
//
// Every query uses ? placeholders and binds the values
// separately. SQLite keeps the values apart from the SQL text, so
// a title like  x' OR '1'='1  is only ever data. Never build SQL
// by joining strings with values.
// ============================================================

#pragma once

#if defined(_WIN32) && __has_include(<winsqlite/winsqlite3.h>)
#include <winsqlite/winsqlite3.h>
#else
#include <sqlite3.h>
#endif

#include "library.h"
#include "storage.h"

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

class DbError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// ------------------------------------------------------------
// RAII Wrappers for the C Handles
// ------------------------------------------------------------
// Copying is deleted: two objects closing one handle would be a
// double free. (Moving could be added; nothing here needs it.)

class Database {
public:
    explicit Database(const std::filesystem::path& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Runs one or more statements with no results (sqlite3_exec)
    void exec(const std::string& sql);
    sqlite3* handle() const { return db_; }

private:
    sqlite3* db_ = nullptr;
};

class Statement {
public:
    Statement(Database& db, std::string_view sql);
    ~Statement();
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    // Placeholders count from 1
    Statement& bind(int index, int value);
    Statement& bind(int index, const std::string& value);
    Statement& bind_null(int index);

    // true when a row is ready to read, false when finished
    bool step();
    // Ready to run again with new values
    void reset();

    int column_int(int index) const;          // columns count from 0
    std::string column_text(int index) const;
    bool column_is_null(int index) const;

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_ = nullptr;
};

// Starts a transaction; rolls it back in the destructor unless
// commit() was called. A return or an exception in between undoes
// every change made inside it.
class Transaction {
public:
    explicit Transaction(Database& db);
    ~Transaction();
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    void commit();

private:
    Database& db_;
    bool done_ = false;
};

// ------------------------------------------------------------
// The Library's Tables
// ------------------------------------------------------------

struct GenreCount {
    std::string genre;
    int count;
};

struct ShelfRow {
    int id;
    std::string title;
    int available;
    int copies;
};

struct TitleRow {
    int id;
    std::string title;
    int year;
};

struct MemberLoans {
    int id;
    std::string name;
    int open_loans;
};

class LibraryDb {
public:
    explicit LibraryDb(const std::filesystem::path& path);

    void create(const std::filesystem::path& schema);   // runs schema.sql
    void import_rows(const LoadResult& rows);            // in one transaction
    int count(std::string_view table);

    // One query each
    std::vector<GenreCount> genre_counts();
    std::vector<ShelfRow> unavailable_books();
    std::vector<TitleRow> search_titles(std::string_view text);
    std::vector<MemberLoans> open_loans_per_member();

    // The task 09 model, built from the tables
    Library load_library();
    void insert_loan(const Loan& loan);
    void mark_returned(const Loan& loan);

    // Stretch 1: the whole borrow in one transaction. Returns the
    // new loan id, or std::nullopt when it was refused (rolled back).
    std::optional<int> borrow_in_transaction(int member_id, int book_id, Date today);
    // Stretch 2
    void add_member_index();

private:
    Database db_;
};
