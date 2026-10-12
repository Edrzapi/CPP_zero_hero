// ============================================================
// db.cpp: Definitions for db.h (starter)
// ============================================================

#include "db.h"

Database::Database(const std::filesystem::path& path)
{
    const std::string name = path.generic_string();   // sqlite3_open takes UTF-8
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
