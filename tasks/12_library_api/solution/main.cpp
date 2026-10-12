// ============================================================
// Task 12: Expose the Library (solution, Core + Stretch)
// ============================================================
// Session 12: HTTP APIs. Builds on task 11. The server runs until
// you stop it with Ctrl+C.
//
// Build and run (from the repository root):
//   cmake --build build --target task12_solution
//   ./build/bin/task12_solution tasks/data [PORT] [TODAY]
//   (PORT defaults to 8080, TODAY to 2026-10-12)
// Then try:
//   curl http://localhost:8080/books?title=the
//   curl -X POST -d "{\"member_id\": 101, \"book_id\": 4}" http://localhost:8080/loans
//
// Stretch, offline:  ./build/bin/task12_solution --summarise FILE
//   prints the summary of a saved Open Library response
// Stretch, online:   ./build/bin/task12_solution --lookup TITLE
//
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 12 -- build/bin/task12_solution tasks/data {port}
//   py -3.13 tasks/check/check.py 12_stretch -- build/bin/task12_solution --summarise tasks/data/openlibrary_dune.json
// ============================================================

#include "api.h"
#include "json.h"
#include "library.h"
#include "openlibrary.h"
#include "server.h"
#include "storage.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

constexpr int DEFAULT_PORT = 8080;
constexpr std::string_view DEFAULT_TODAY = "2026-10-12";

int summarise_file(const std::filesystem::path& path)
{
    std::ifstream file{path, std::ios::binary};
    if (!file) {
        std::cerr << "cannot read " << path.generic_string() << '\n';
        return 1;
    }
    std::ostringstream text;
    text << file.rdbuf();
    for (const std::string& line : summarise(text.str())) {
        std::cout << line << '\n';
    }
    return 0;
}

int lookup(const std::string& title)
{
    try {
        for (const std::string& line : summarise(search_open_library(title, 3))) {
            std::cout << line << '\n';
        }
    } catch (const std::exception& error) {
        std::cout << "Open Library did not answer: " << error.what() << '\n';
    }
    return 0;
}

int run(int argc, char* argv[])
{
    if (argc == 3 && std::string_view{argv[1]} == "--summarise") {
        return summarise_file(argv[2]);
    }
    if (argc == 3 && std::string_view{argv[1]} == "--lookup") {
        return lookup(argv[2]);
    }

    const std::filesystem::path data = argc > 1 ? std::filesystem::path{argv[1]} : "tasks/data";
    const int port = argc > 2 ? std::stoi(argv[2]) : DEFAULT_PORT;
    const std::optional<Date> today = parse_iso_date(argc > 3 ? argv[3] : DEFAULT_TODAY);
    if (!today) {
        std::cerr << "today must be a date YYYY-MM-DD\n";
        return 2;
    }

    // Every start loads a fresh library from the CSV files, so every
    // run of the check sees the same data
    LibraryApi api{load_all(data), *today};
    serve(port, [&api](const Request& request) { return api.handle(request); },
          [port, &today] {
              // flush, so the line appears at once even when stdout is a file
              std::cout << "Library API on http://localhost:" << port << " (today is " << to_iso(*today) << ")"
                        << std::endl;
          });
    return 0;
}

int main(int argc, char* argv[])
{
    // A missing data folder or a folder that cannot be written ends
    // the program with a message, not an unhandled exception
    try {
        return run(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
