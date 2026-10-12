// ============================================================
// api.cpp: Definitions for api.h (starter)
// ============================================================

#include "api.h"

#include "json.h"

#include <utility>

LibraryApi::LibraryApi(Library library, Date today) : library_{std::move(library)}, today_{today} {}

// TODO Core 2 to 5: route the request.
//   GET /books               200, every book as a JSON array, by id
//   GET /books?title=TEXT    200, matching books (decode %XX in TEXT)
//   POST /loans              201 the new loan, or 400 / 404 / 409
//   anything else            404 {"error": "not found"}
// Book objects: id, title, author, year, genre, format, copies,
// available. Build the JSON by hand with json_quote for text.
Response LibraryApi::handle([[maybe_unused]] const Request& request)
{
    return {404, "{\"error\": \"not implemented\"}"};
}
