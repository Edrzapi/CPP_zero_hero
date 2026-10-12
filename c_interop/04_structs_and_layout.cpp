// ============================================================
// Structs, Memory Layout and Trivially Copyable Types
// ============================================================
// What a struct looks like in memory, why member order changes
// its size, and which C++ types can safely be shared with C,
// copied with memcpy or written to a file as raw bytes.
//
// "POD" (plain old data) is the C++98 name for C-compatible
// types. C++20 splits the idea into two precise properties:
//   trivially copyable  can be copied with memcpy
//   standard layout     has the same layout a C compiler would use
//
// Build and run:
//   cmake --build build --target c_interop_04_structs_and_layout
//   ./build/bin/c_interop_04_structs_and_layout
// ============================================================

#include <cstddef>      // offsetof, std::max_align_t
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>     // std::size
#include <string>
#include <type_traits>

// ------------------------------------------------------------
// Padding and Alignment
// ------------------------------------------------------------
// Each type has an alignment: an int (4 bytes) usually has to
// start at an address divisible by 4. The compiler inserts
// padding bytes between members to keep each one aligned, and at
// the end so that arrays of the struct stay aligned.
// Members are laid out in declaration order (for members with the
// same access), so ordering them largest first often saves space.

struct Wasteful {
    char a;          // 1 byte, then 3 bytes padding
    std::int32_t b;  // 4 bytes
    char c;          // 1 byte, then 3 bytes padding at the end
};

struct Compact {
    std::int32_t b;  // 4 bytes
    char a;          // 1 byte
    char c;          // 1 byte, then 2 bytes padding
};

void padding()
{
    std::cout << "=== Padding ===\n";
    std::cout << "sizeof(Wasteful) = " << sizeof(Wasteful) << '\n';   // 12 on mainstream platforms
    std::cout << "sizeof(Compact)  = " << sizeof(Compact) << '\n';    // 8

    // offsetof works on standard-layout types (a C macro).
    std::cout << "offsetof(Wasteful, b) = " << offsetof(Wasteful, b) << '\n';   // 4
    std::cout << "offsetof(Wasteful, c) = " << offsetof(Wasteful, c) << '\n';   // 8

    std::cout << "alignof(int32_t)  = " << alignof(std::int32_t) << '\n';
    std::cout << "alignof(double)   = " << alignof(double) << '\n';
    std::cout << "alignof(max_align_t) = " << alignof(std::max_align_t) << '\n';

    // #pragma pack(1) removes padding (common in network and file
    // headers) at the cost of slower or even faulting unaligned
    // access on some CPUs. Prefer reading fields individually.
}

// ------------------------------------------------------------
// Trivially Copyable and Standard Layout
// ------------------------------------------------------------
// Trivially copyable: no user-provided copy/move/destructor, no
// virtual functions. Bytes can be memcpy'd into another object.
// Standard layout: all non-static members have the same access,
// no virtual functions, no virtual bases, and a few rules on base
// classes. Matches the C layout, so it can cross into C code.

struct Sample {             // a C-compatible struct
    std::int32_t id;
    double reading;
};

struct WithString {         // std::string owns heap memory
    std::int32_t id;
    std::string label;
};

class WithVirtual {         // the hidden vtable pointer changes the layout
public:
    virtual ~WithVirtual() = default;
    int value = 0;
};

class MixedAccess {         // public and private data members
public:
    int a = 0;
    int get_b() const { return b; }
private:
    int b = 0;
};

void type_properties()
{
    std::cout << "\n=== Type Properties ===\n" << std::boolalpha;

    std::cout << "Sample      trivially copyable: " << std::is_trivially_copyable_v<Sample>
              << ", standard layout: " << std::is_standard_layout_v<Sample> << '\n';
    std::cout << "WithString  trivially copyable: " << std::is_trivially_copyable_v<WithString>
              << ", standard layout: " << std::is_standard_layout_v<WithString> << '\n';
    std::cout << "WithVirtual trivially copyable: " << std::is_trivially_copyable_v<WithVirtual>
              << ", standard layout: " << std::is_standard_layout_v<WithVirtual> << '\n';
    std::cout << "MixedAccess trivially copyable: " << std::is_trivially_copyable_v<MixedAccess>
              << ", standard layout: " << std::is_standard_layout_v<MixedAccess> << '\n';

    // WithString's answers depend on the library's std::string layout:
    // standard layout is true with libstdc++, libc++ and MSVC Release,
    // but false in MSVC Debug builds (iterator-debugging members).
    // Only rely on these traits for types you control, like Sample.

    // A static_assert documents and enforces the assumption in code
    // that depends on it, such as a struct written to a binary file.
    static_assert(std::is_trivially_copyable_v<Sample>, "Sample is written with memcpy");
    static_assert(std::is_standard_layout_v<Sample>, "Sample is shared with C code");
}

// ------------------------------------------------------------
// memcpy and Binary Serialisation
// ------------------------------------------------------------
// For trivially copyable types, copying the bytes copies the
// value. For anything else it is undefined behaviour: memcpy of a
// WithString copies the internal pointer, and both copies later
// free the same heap block.
//
// Raw bytes are also not portable between machines: padding,
// endianness and type sizes may differ. Fine for a cache file on
// the same machine, not for a network protocol.

void byte_copies()
{
    std::cout << "\n=== memcpy of a Trivially Copyable Type ===\n";

    Sample original{42, 98.6};
    unsigned char buffer[sizeof(Sample)];
    std::memcpy(buffer, &original, sizeof original);   // "serialise"

    Sample restored{};
    std::memcpy(&restored, buffer, sizeof restored);   // "deserialise"
    std::cout << "restored: id " << restored.id << ", reading " << restored.reading << '\n';
}

// ------------------------------------------------------------
// Passing a Struct to a C Function
// ------------------------------------------------------------
// Pretend this function lives in a C library. Because Sample is
// standard layout and uses only C types, C and C++ agree on where
// every member is.

extern "C" double c_average(const Sample* samples, std::size_t count)
{
    if (count == 0) return 0.0;
    double total = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        total += samples[i].reading;
    }
    return total / static_cast<double>(count);
}

void pass_to_c()
{
    std::cout << "\n=== Passing Structs to C ===\n";
    Sample samples[] = {{1, 10.0}, {2, 20.0}, {3, 30.0}};
    std::cout << "c_average = " << c_average(samples, std::size(samples)) << '\n';   // 20
}

// ------------------------------------------------------------
// Bit-Fields
// ------------------------------------------------------------
// Pack small values into fewer bits. Order and padding of
// bit-fields are implementation-defined, so never rely on them
// for a wire format; use explicit masks and shifts there.

struct Flags {
    unsigned ready : 1;
    unsigned error : 1;
    unsigned level : 3;   // 0 to 7
};

void bit_fields()
{
    std::cout << "\n=== Bit-Fields ===\n";
    Flags f{1, 0, 5};
    std::cout << "ready " << f.ready << ", error " << f.error << ", level " << f.level
              << ", sizeof(Flags) = " << sizeof(Flags) << '\n';
}

int main()
{
    padding();
    type_properties();
    byte_copies();
    pass_to_c();
    bit_fields();
}
