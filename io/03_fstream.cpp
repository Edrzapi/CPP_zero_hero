// ============================================================
// File Streams: Reading and Writing Files
// ============================================================
// std::ofstream writes, std::ifstream reads, std::fstream does
// both. They are RAII types: the file closes when the stream
// object is destroyed, so there is no try-with-resources or using
// block to remember. All files here go in the system temp folder
// and are removed at the end.
//
// Build and run:
//   cmake --build build --target io_03_fstream
//   ./build/bin/io_03_fstream
// ============================================================

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>      // std::istreambuf_iterator
#include <string>
#include <type_traits>
#include <vector>

namespace fs = std::filesystem;

// ------------------------------------------------------------
// Writing Text
// ------------------------------------------------------------
// Open modes: std::ios::out (default for ofstream, truncates),
// std::ios::app (append), std::ios::binary (no newline translation
// on Windows), std::ios::in (default for ifstream).

void write_text(const fs::path& path)
{
    std::cout << "=== Writing Text ===\n";
    std::ofstream out{path};
    if (!out) {                                  // always check the open succeeded
        std::cerr << "  cannot open " << path << '\n';
        return;
    }
    out << "name,score\n";
    out << "Ada,91\n";
    out << "Alan,88\n";
    std::cout << "  wrote " << path.filename() << '\n';
}   // closed (and flushed) here

void append_text(const fs::path& path)
{
    std::ofstream out{path, std::ios::app};
    out << "Grace,95\n";
}

// ------------------------------------------------------------
// Reading Text
// ------------------------------------------------------------
// Line by line with std::getline. The loop condition is the read
// itself: it becomes false at end of file or on an error. Testing
// eof() BEFORE reading is a classic bug that processes the last
// line twice.

void read_lines(const fs::path& path)
{
    std::cout << "\n=== Reading Line by Line ===\n";
    std::ifstream in{path};
    if (!in) {
        std::cerr << "  cannot open " << path << '\n';
        return;
    }
    std::string line;
    int number = 0;
    while (std::getline(in, line)) {
        std::cout << "  " << ++number << ": " << line << '\n';
    }
}

// Read the whole file into one string.
std::string read_all(const fs::path& path)
{
    std::ifstream in{path, std::ios::binary};
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
}

// ------------------------------------------------------------
// Binary Files
// ------------------------------------------------------------
// write()/read() copy raw bytes. Only safe for trivially copyable
// types (see c_interop/04_structs_and_layout.cpp), and the file is
// only portable between machines with the same sizes, padding and
// endianness.

struct Record {
    std::int32_t id;
    double balance;
};
static_assert(std::is_trivially_copyable_v<Record>);

void binary_round_trip(const fs::path& path)
{
    std::cout << "\n=== Binary Round Trip ===\n";

    const std::vector<Record> records{{1, 10.5}, {2, 99.25}, {3, -4.0}};
    {
        std::ofstream out{path, std::ios::binary};
        out.write(reinterpret_cast<const char*>(records.data()),
                  static_cast<std::streamsize>(records.size() * sizeof(Record)));
    }

    std::vector<Record> loaded(records.size());
    {
        std::ifstream in{path, std::ios::binary};
        in.read(reinterpret_cast<char*>(loaded.data()),
                static_cast<std::streamsize>(loaded.size() * sizeof(Record)));
        std::cout << "  read " << in.gcount() << " bytes\n";
    }
    for (const auto& r : loaded) {
        std::cout << "  id " << r.id << ", balance " << r.balance << '\n';
    }
    std::cout << "  file size on disk: " << fs::file_size(path) << " bytes ("
              << records.size() << " x sizeof(Record) = " << sizeof(Record) << ")\n";
}

// ------------------------------------------------------------
// Errors as Exceptions
// ------------------------------------------------------------
// Streams report errors through state flags by default. exceptions()
// switches them to throwing std::ios_base::failure instead.

void missing_file()
{
    std::cout << "\n=== Opening a File That Does Not Exist ===\n";
    std::ifstream in{fs::temp_directory_path() / "cppzh_does_not_exist.txt"};
    std::cout << "  is_open() = " << std::boolalpha << in.is_open() << ", good() = " << in.good() << '\n';
}

int main()
{
    const fs::path text_file = fs::temp_directory_path() / "cppzh_scores.csv";
    const fs::path binary_file = fs::temp_directory_path() / "cppzh_records.bin";

    write_text(text_file);
    append_text(text_file);
    read_lines(text_file);
    // 35 on Linux and macOS, 39 on Windows: a text-mode ofstream on
    // Windows writes each '\n' as "\r\n", and read_all opens in binary.
    std::cout << "  whole file is " << read_all(text_file).size() << " bytes\n";

    binary_round_trip(binary_file);
    missing_file();

    fs::remove(text_file);
    fs::remove(binary_file);
}
