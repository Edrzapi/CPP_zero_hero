// ============================================================
// RAII: Resource Acquisition Is Initialisation
// ============================================================
// The single most important C++ idiom. Tie every resource (heap
// memory, a file, a lock, a socket, a database connection) to an
// object: acquire it in the constructor, release it in the
// destructor. Because destructors of stack objects ALWAYS run when
// the scope ends, normally or by exception, the resource can
// never leak.
//
// Compared with other languages:
//   Java   try-with-resources + AutoCloseable
//   C#     using + IDisposable
//   Python with + context managers
//   C++    nothing to write at the call site: it is automatic
//
// Build and run:
//   cmake --build build --target memory_02_raii
//   ./build/bin/memory_02_raii
// ============================================================

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

// ------------------------------------------------------------
// The C Way: Manual Clean-Up on Every Path
// ------------------------------------------------------------
// Every early return must remember fclose. Add an exception (or
// a new early return next year) and the file handle leaks.

bool c_style_write(const std::string& path, bool fail_halfway)
{
    std::FILE* file = std::fopen(path.c_str(), "w");
    if (file == nullptr) {
        return false;
    }
    std::fputs("first line\n", file);
    if (fail_halfway) {
        std::fclose(file);   // easy to forget
        return false;
    }
    std::fputs("second line\n", file);
    std::fclose(file);
    return true;
}

// ------------------------------------------------------------
// The RAII Way: a File Handle Class
// ------------------------------------------------------------
// The destructor closes the file. The class is move-only: two
// objects must never own the same FILE*. (std::fstream already
// does all this; the class shows how it works inside.)

class File {
public:
    File(const std::string& path, const char* mode) : handle_{std::fopen(path.c_str(), mode)}
    {
        if (handle_ == nullptr) {
            throw std::runtime_error{"cannot open " + path};
        }
        std::cout << "  opened " << path << '\n';
    }

    ~File()
    {
        if (handle_ != nullptr) {
            std::fclose(handle_);
            std::cout << "  closed file in destructor\n";
        }
    }

    // No copies: which copy would close the file?
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    // Moves transfer ownership and leave the source empty.
    File(File&& other) noexcept : handle_{std::exchange(other.handle_, nullptr)} {}
    File& operator=(File&& other) noexcept
    {
        if (this != &other) {
            if (handle_ != nullptr) std::fclose(handle_);
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }

    void write(const std::string& text)
    {
        if (std::fputs(text.c_str(), handle_) == EOF) {
            throw std::runtime_error{"write failed"};
        }
    }

private:
    std::FILE* handle_;
};

void raii_write(const std::string& path, bool fail_halfway)
{
    File file{path, "w"};
    file.write("first line\n");
    if (fail_halfway) {
        throw std::runtime_error{"something went wrong halfway"};
    }
    file.write("second line\n");
}   // file closed here on every path

// ------------------------------------------------------------
// Scope Guards
// ------------------------------------------------------------
// For one-off clean-up without writing a class, a tiny generic
// guard runs a lambda in its destructor. (Libraries call this
// scope_exit or finally; std::experimental::scope_exit exists in
// the Library Fundamentals TS but is not yet standard.)

template <typename F>
class ScopeGuard {
public:
    explicit ScopeGuard(F action) : action_{std::move(action)} {}
    ~ScopeGuard() { action_(); }
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    F action_;
};

void scope_guard_demo()
{
    std::cout << "\n=== Scope Guard ===\n";
    ScopeGuard guard{[] { std::cout << "  clean-up ran at end of scope\n"; }};
    std::cout << "  doing work...\n";
}

int main()
{
    const auto path = (std::filesystem::temp_directory_path() / "cppzh_raii_demo.txt").string();

    std::cout << "=== C Style ===\n";
    std::cout << "  c_style_write returned " << std::boolalpha << c_style_write(path, true) << '\n';

    std::cout << "\n=== RAII, Normal Path ===\n";
    raii_write(path, false);

    std::cout << "\n=== RAII, Exception Path ===\n";
    try {
        raii_write(path, true);
    } catch (const std::exception& e) {
        std::cout << "  caught: " << e.what() << " (the file was already closed)\n";
    }

    scope_guard_demo();

    std::filesystem::remove(path);
}
