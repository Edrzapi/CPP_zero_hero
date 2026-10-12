// ============================================================
// db.h: the Library in SQLite (starter)
// ============================================================
// The header and the library are already found and linked for
// you (see tasks/CMakeLists.txt): winsqlite3 on Windows, the
// system libsqlite3 elsewhere. Database is a finished RAII
// wrapper for the connection, so you can see the pattern; write
// the same kind of wrapper for a statement (sqlite3_prepare_v2,
// sqlite3_bind_*, sqlite3_step, sqlite3_column_*, and
// sqlite3_finalize in the destructor), then the queries.
//
// Parameterised queries only: ? placeholders and sqlite3_bind_*.
// Never build SQL by joining strings with values.
// ============================================================

#pragma once

#if defined(_WIN32) && __has_include(<winsqlite/winsqlite3.h>)
#include <winsqlite/winsqlite3.h>
#else
#include <sqlite3.h>
#endif

#include <filesystem>
#include <stdexcept>
#include <string>

class DbError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Opens the database in the constructor, closes it in the
// destructor. Copying is deleted: two objects closing one handle
// would be a double free.
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

// TODO: class Statement (RAII for sqlite3_stmt*)
// TODO: class LibraryDb with create(schema), import_rows(rows),
//       count(table), the four queries, load_library(),
//       insert_loan(loan) and mark_returned(loan)
