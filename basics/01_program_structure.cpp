// ============================================================
// Program Structure: From Source File to Running Program
// ============================================================
// The smallest useful C++ program, what each line does, and
// how to build it by hand before CMake gets involved.
//
// Build by hand (one file, no CMake):
//   g++     -std=c++20 -Wall -Wextra 01_program_structure.cpp -o hello
//   clang++ -std=c++20 -Wall -Wextra 01_program_structure.cpp -o hello
//   cl /std:c++20 /EHsc /W4 01_program_structure.cpp      (MSVC)
//
// Build and run with CMake:
//   cmake --build build --target basics_01_program_structure
//   ./build/bin/basics_01_program_structure first second
// ============================================================

// ------------------------------------------------------------
// #include
// ------------------------------------------------------------
// #include is a preprocessor instruction: the contents of the
// named header are pasted in before the compiler sees the file.
// <iostream> declares std::cout, std::cin, std::cerr.
// There is no import system like Java or Python (C++20 modules
// exist but tooling support is still uneven; see compilation/).

#include <iostream>
#include <string>

// ------------------------------------------------------------
// Namespaces
// ------------------------------------------------------------
// Everything in the standard library lives in namespace std.
// "using namespace std;" at file scope saves typing but pulls
// hundreds of names into the global namespace and can clash
// with your own names. Never put it in a header. In a .cpp
// file, prefer targeted using-declarations or write std::.

namespace greeting {

// A function declared inside a namespace is called with the
// qualified name greeting::make_message(...).
std::string make_message(const std::string& name)
{
    return "Hello, " + name + "!";
}

}  // namespace greeting

// ------------------------------------------------------------
// main
// ------------------------------------------------------------
// Execution starts at main. Two standard signatures:
//   int main()
//   int main(int argc, char* argv[])
// argv[0] is the program name; argv[1..argc-1] are the arguments.
//
// The return value goes to the operating system: 0 means
// success, anything else means failure. main is the only
// function where falling off the end returns 0 automatically.

int main(int argc, char* argv[])
{
    std::cout << "=== Hello, World ===\n";

    // std::cout is an output stream; << inserts values into it.
    // "\n" ends the line. std::endl also flushes the buffer,
    // which is slower; only use it when you need the flush.
    std::cout << greeting::make_message("World") << '\n';

    // A using-declaration brings in one name, not the whole namespace.
    using std::cout;
    cout << "Using-declaration: cout without the std:: prefix\n";

    std::cout << "\n=== Command Line Arguments ===\n";
    std::cout << "argc = " << argc << '\n';
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "] = " << argv[i] << '\n';
    }

    // std::cerr is unbuffered and goes to the error stream.
    // Redirect stdout to a file and this still shows on screen.
    std::cerr << "(this line goes to std::cerr)\n";

    // ------------------------------------------------------------
    // Comments
    // ------------------------------------------------------------
    // Line comments start with //.
    /* Block comments use slash-star and do not nest,
       so commenting out code that already holds a block
       comment breaks. Use #if 0 ... #endif for that. */

    return 0;  // optional in main, shown here for clarity
}
