// ============================================================
// openlibrary.h: Stretch, Consuming a Public API
// ============================================================
// https://openlibrary.org/search.json?title=dune
// Two halves, kept apart on purpose:
//   * summarise() and first_match() turn response TEXT into
//     lines or JSON. No network, so they are checked against
//     the recorded data/openlibrary_dune.json
//   * search_open_library() does the HTTP call. It needs the
//     network, so it is never part of a check
// ============================================================

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

class OpenLibraryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// The raw JSON text of a title search. Throws OpenLibraryError
// when the call fails or takes more than 10 seconds.
std::string search_open_library(const std::string& title, int limit);

// "Open Library search: N results", then one line per result.
// Throws JsonError when the text is not JSON.
std::vector<std::string> summarise(std::string_view json_text);

// The first result as a JSON object for the API, or "null"
std::string first_match(std::string_view json_text);
