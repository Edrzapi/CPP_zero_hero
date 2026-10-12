// ============================================================
// Task 08: Persistence (solution, Core + Stretch)
// ============================================================
// Session 8: file I/O (see io/03_fstream.cpp and
// io/04_filesystem.cpp). Builds on task 07: the data now comes
// from the files in the data folder, and the library is saved to
// CSV and JSON in a folder called out (in the current folder).
// Core reads the CSV back; reading the JSON back is the Stretch.
//
// Build and run (from the repository root):
//   cmake --build build --target task08_solution
//   ./build/bin/task08_solution tasks/data              Core (checked)
//   ./build/bin/task08_solution tasks/data --stretch    Core, then Stretch
// DATA_DIR defaults to tasks/data.
// Check (from the repository root):
//   py -3.13 tasks/check/check.py 08 -- build/bin/task08_solution tasks/data
// ============================================================

#include "library.h"
#include "storage.h"

#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

const fs::path OUT = "out";

// Same ids with the same field values, in the same order
bool same_books(const Library& a, const Library& b)
{
    const auto left = a.all_books();
    const auto right = b.all_books();
    if (left.size() != right.size()) return false;
    for (std::size_t i = 0; i < left.size(); ++i) {
        const Book& x = *left[i];
        const Book& y = *right[i];
        // make_tuple bundles the fields: one == compares them all in order
        if (std::make_tuple(x.id(), x.title(), x.author(), x.year(), x.genre(), x.copies(), x.length()) !=
                std::make_tuple(y.id(), y.title(), y.author(), y.year(), y.genre(), y.copies(), y.length()) ||
            x.format_name() != y.format_name()) {
            return false;
        }
    }
    return true;
}

bool same_members(const Library& a, const Library& b)
{
    const auto left = a.all_members();
    const auto right = b.all_members();
    if (left.size() != right.size()) return false;
    for (std::size_t i = 0; i < left.size(); ++i) {
        const Member& x = *left[i];
        const Member& y = *right[i];
        if (std::make_tuple(x.id(), x.name(), x.email(), x.type(), x.joined(), x.loans()) !=
            std::make_tuple(y.id(), y.name(), y.email(), y.type(), y.joined(), y.loans())) {
            return false;
        }
    }
    return true;
}

// Load a file that might be missing or broken, and say what happened
void try_load(const std::string& name, const std::function<LoadResult(const fs::path&)>& loader,
              const fs::path& path)
{
    std::cout << "Loading " << name << '\n';
    LoadResult result;
    try {
        result = loader(path);
    } catch (const FileNotFoundError&) {
        std::cout << "  file not found, starting with an empty catalogue\n";
        return;
    } catch (const StorageError& error) {
        std::cout << "  " << error.what() << ", starting with an empty catalogue\n";
        return;
    }
    for (const std::string& problem : result.problems) {
        std::cout << "  " << problem << '\n';
    }
    std::cout << "  loaded " << result.books.size() << " books, skipped " << result.skipped << " lines\n";
}

int run(int argc, char* argv[])
{
    std::vector<std::string> args;   // the arguments that are not --flags
    bool stretch = false;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--stretch") stretch = true;
        else if (!arg.starts_with("--")) args.emplace_back(arg);
    }
    const fs::path data = args.empty() ? fs::path{"tasks/data"} : fs::path{args[0]};

    // --------------------------------------------------------
    // Core 1: load the real data from disk
    // --------------------------------------------------------
    LoadResult loaded = load_books_csv(data / "books.csv");
    LoadResult members = load_members_csv(data / "members.csv");
    std::cout << "Loaded " << loaded.books.size() << " books from books.csv\n";
    std::cout << "Loaded " << members.members.size() << " members from members.csv\n";
    loaded.members = std::move(members.members);
    Library library = build_library(std::move(loaded));

    // --------------------------------------------------------
    // Core 2 and 3: save as CSV and JSON, load the CSV back
    // --------------------------------------------------------
    // Paths are shown with / whatever the operating system.
    save_books_csv(library, OUT / "books.csv");
    std::cout << "Saved " << library.book_count() << " books to out/books.csv\n";
    save_json(library, OUT / "library.json");
    std::cout << "Saved " << library.book_count() << " books and " << library.member_count()
              << " members to out/library.json\n";

    const Library from_csv = build_library(load_books_csv(OUT / "books.csv"));
    std::cout << "Reloaded out/books.csv: " << from_csv.book_count() << " books, "
              << (same_books(library, from_csv) ? "same as before" : "DIFFERENT") << '\n';

    // --------------------------------------------------------
    // Core 4 and 5: a missing file and a corrupt CSV
    // --------------------------------------------------------
    try_load("missing.csv", load_books_csv, data / "missing.csv");
    try_load("books_corrupt.csv", load_books_csv, data / "books_corrupt.csv");

    // --------------------------------------------------------
    // Stretch: read the JSON back, cope with broken JSON, and
    // keep loans across a restart
    // --------------------------------------------------------
    if (stretch) {
        std::cout << "Stretch\n";
        const Library from_json = build_library(load_json(OUT / "library.json"));
        const bool same = same_books(library, from_json) && same_members(library, from_json);
        std::cout << "Reloaded out/library.json: " << from_json.book_count() << " books and "
                  << from_json.member_count() << " members, " << (same ? "same as before" : "DIFFERENT") << '\n';
        try_load("library_corrupt.json", load_json, data / "library_corrupt.json");

        library.borrow(101, 4);
        library.borrow(101, 7);
        save_json(library, OUT / "library.json");
        const Library restarted = build_library(load_json(OUT / "library.json"));
        std::cout << "after restart: " << restarted.member(101).to_string() << '\n';
        std::cout << "after restart: " << restarted.book(4).to_string() << '\n';
    }
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
