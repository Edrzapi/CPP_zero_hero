// ============================================================
// std::filesystem: Paths, Directories and Files (C++17)
// ============================================================
// Portable path handling and directory operations, replacing
// POSIX opendir/stat and the Win32 FindFirstFile family.
// Everything happens inside a scratch folder in the system temp
// directory, which is deleted at the end.
//
// Build and run:
//   cmake --build build --target io_04_filesystem
//   ./build/bin/io_04_filesystem
// ============================================================

#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

// ------------------------------------------------------------
// Paths
// ------------------------------------------------------------
// operator/ joins path pieces with the right separator.
// Decomposition functions pull a path apart without string hacks.

void paths()
{
    std::cout << "=== Paths ===\n";

    fs::path p = fs::path{"reports"} / "2026" / "summary.final.txt";
    std::cout << "  path        : " << p.generic_string() << '\n';    // always forward slashes
    std::cout << "  filename    : " << p.filename() << '\n';
    std::cout << "  stem        : " << p.stem() << '\n';
    std::cout << "  extension   : " << p.extension() << '\n';
    std::cout << "  parent_path : " << p.parent_path().generic_string() << '\n';

    fs::path changed = p;
    changed.replace_extension(".csv");
    std::cout << "  replaced ext: " << changed.filename() << '\n';
    std::cout << "  is_absolute : " << std::boolalpha << p.is_absolute() << '\n';
    // Printing a path with << quotes it, which is why some lines show "...".
}

// ------------------------------------------------------------
// Creating, Listing, Inspecting
// ------------------------------------------------------------

void create_files(const fs::path& root)
{
    fs::create_directories(root / "logs" / "old");   // like mkdir -p
    for (const char* name : {"a.txt", "b.txt", "notes.md"}) {
        std::ofstream{root / name} << "sample content for " << name << '\n';
    }
    std::ofstream{root / "logs" / "app.log"} << "log line\n";
    std::ofstream{root / "logs" / "old" / "app.1.log"} << "older log line\n";
}

void listing(const fs::path& root)
{
    std::cout << "\n=== directory_iterator (one level) ===\n";
    for (const auto& entry : fs::directory_iterator{root}) {
        std::cout << "  " << (entry.is_directory() ? "[dir]  " : "[file] ")
                  << entry.path().filename().string() << '\n';
    }
    // Directory order is unspecified: sort the entries if you need a stable order.

    std::cout << "\n=== recursive_directory_iterator (.log files) ===\n";
    for (const auto& entry : fs::recursive_directory_iterator{root}) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            std::cout << "  " << fs::relative(entry.path(), root).generic_string()
                      << " (" << entry.file_size() << " bytes)\n";
        }
    }
}

// ------------------------------------------------------------
// Copy, Rename, Remove
// ------------------------------------------------------------

void modifying(const fs::path& root)
{
    std::cout << "\n=== Copy, Rename, Remove ===\n";
    fs::copy_file(root / "a.txt", root / "a_copy.txt");
    fs::rename(root / "b.txt", root / "b_renamed.txt");
    fs::remove(root / "notes.md");

    std::cout << "  a_copy.txt exists    : " << fs::exists(root / "a_copy.txt") << '\n';
    std::cout << "  b.txt exists         : " << fs::exists(root / "b.txt") << '\n';
    std::cout << "  b_renamed.txt exists : " << fs::exists(root / "b_renamed.txt") << '\n';
    std::cout << "  notes.md exists      : " << fs::exists(root / "notes.md") << '\n';
}

// ------------------------------------------------------------
// Two Error-Handling Styles
// ------------------------------------------------------------
// Most functions come in two overloads: one throws
// fs::filesystem_error, the other takes std::error_code& and
// reports through it instead. Choose per call site.

void error_styles(const fs::path& root)
{
    std::cout << "\n=== Throwing vs error_code ===\n";

    try {
        std::cout << fs::file_size(root / "missing.txt");   // throws before printing
    } catch (const fs::filesystem_error& e) {
        std::cout << "  threw filesystem_error: " << e.code().message() << '\n';
    }

    std::error_code ec;
    auto size = fs::file_size(root / "missing.txt", ec);
    if (ec) {
        std::cout << "  error_code overload: " << ec.message() << " (size returned: "
                  << static_cast<long long>(size) << ")\n";
    }
}

int main()
{
    paths();

    const fs::path root = fs::temp_directory_path() / "cppzh_filesystem_demo";
    fs::remove_all(root);   // start clean if a previous run was interrupted
    create_files(root);

    listing(root);
    modifying(root);
    error_styles(root);

    auto removed = fs::remove_all(root);
    std::cout << "\n  cleaned up " << removed << " files and folders\n";
}
