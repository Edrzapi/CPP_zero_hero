// ============================================================
// api.h: the Library over HTTP, JSON In and Out
// ============================================================
// Core endpoints:
//   GET  /books              every book, by id
//   GET  /books?title=TEXT   books whose title contains TEXT (any case)
//   POST /loans              {"member_id": 101, "book_id": 4}
// Stretch endpoints:
//   GET  /books/ID               one book
//   GET  /books/ID/openlibrary   Open Library's first match for it
// Anything else is 404 {"error": "not found"}.
//
// LibraryApi knows nothing about sockets: it takes a Request and
// returns a Response. That keeps it easy to test, and server.cpp
// knows nothing about libraries.
// ============================================================

#pragma once

#include "library.h"
#include "server.h"

#include <string>

class LibraryApi {
public:
    LibraryApi(Library library, Date today);

    Response handle(const Request& request);

private:
    Response list_books(const std::string& query);
    Response get_book(int book_id);
    Response create_loan(const std::string& body);
    Response open_library(int book_id);

    std::string book_json(const Book& book) const;

    Library library_;
    Date today_;
};
