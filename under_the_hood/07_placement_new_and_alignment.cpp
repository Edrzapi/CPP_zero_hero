// ============================================================
// Placement new, Object Lifetime and Alignment
// ============================================================
// "new T" does two things: allocate memory, then construct a T in
// it. Placement new does ONLY the second step, in memory you
// already have. That is how std::vector, std::optional and memory
// pools work inside: reserve raw storage, construct objects in it
// when needed, destroy them by hand later.
//
// Build and run:
//   cmake --build build --target under_the_hood_07_placement_new_and_alignment
//   ./build/bin/under_the_hood_07_placement_new_and_alignment
// ============================================================

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>    // std::construct_at, std::destroy_at
#include <new>       // placement new, std::launder, hardware_destructive_interference_size
#include <stdexcept>
#include <string>
#include <utility>
#include <version>

#if defined(_MSC_VER)
// C4324: "structure was padded due to alignment specifier". That
// padding is exactly what this file demonstrates.
#pragma warning(disable : 4324)
#endif

class Widget {
public:
    explicit Widget(std::string name) : name_{std::move(name)} { std::cout << "  construct " << name_ << '\n'; }
    ~Widget() { std::cout << "  destroy   " << name_ << '\n'; }
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// ------------------------------------------------------------
// Placement new and an Explicit Destructor Call
// ------------------------------------------------------------
// The storage must be big enough AND suitably aligned for the
// type, hence alignas(Widget). Because the compiler did not create
// the object, it will not destroy it: call the destructor yourself.
// Never "delete" an object created with placement new.

void placement_new()
{
    std::cout << "=== Placement new ===\n";

    alignas(Widget) std::byte storage[sizeof(Widget)];   // raw bytes, no Widget yet

    Widget* w = new (storage) Widget{"in-place"};      // construct in existing memory
    std::cout << "  using " << w->name() << " at the address of storage? " << std::boolalpha
              << (static_cast<void*>(w) == static_cast<void*>(storage)) << '\n';
    w->~Widget();                                       // destroy, memory stays

    // C++20 spells the same two steps as library functions.
    Widget* again = std::construct_at(reinterpret_cast<Widget*>(storage), "construct_at");
    std::destroy_at(again);

    // std::launder (C++17) is needed in a few corner cases when you
    // reach a new object through a pointer to the OLD object in the
    // same storage (const members, references, or storage reuse with
    // a different type). Use the pointer returned by new / construct_at
    // and you rarely need it.
}

// ------------------------------------------------------------
// A Tiny Fixed-Capacity Buffer: How Containers Do It
// ------------------------------------------------------------
// Storage for N objects, but objects exist only once pushed.

template <typename T, std::size_t N>
class StaticVector {
public:
    ~StaticVector()
    {
        for (std::size_t i = 0; i < size_; ++i) std::destroy_at(slot(i));
    }

    void push_back(const T& value)
    {
        if (size_ == N) throw std::length_error{"StaticVector full"};
        std::construct_at(raw_slot(size_), value);   // no object there yet: use the raw address
        ++size_;
    }

    T& operator[](std::size_t i) { return *slot(i); }
    std::size_t size() const { return size_; }

    StaticVector() = default;
    StaticVector(const StaticVector&) = delete;
    StaticVector& operator=(const StaticVector&) = delete;

private:
    // The address where element i lives (or will live).
    T* raw_slot(std::size_t i) { return reinterpret_cast<T*>(storage_ + i * sizeof(T)); }

    // A pointer to a LIVE element. The storage pointer was never a
    // pointer to a T, so std::launder is the formally correct way to
    // obtain one to the object constructed there.
    T* slot(std::size_t i) { return std::launder(raw_slot(i)); }

    alignas(T) std::byte storage_[N * sizeof(T)];
    std::size_t size_ = 0;
};

// ------------------------------------------------------------
// Alignment
// ------------------------------------------------------------
// alignof(T): the address multiple T needs. alignas(N): request a
// stricter alignment. Since C++17, plain "new" respects
// over-aligned types (it calls the aligned operator new).
//
// Cache lines (typically 64 bytes): two threads writing different
// variables on the SAME cache line slow each other down ("false
// sharing"). Aligning each to its own line fixes it.

struct SharedCounters {                 // both counters likely on one cache line
    std::int64_t a = 0;
    std::int64_t b = 0;
};

struct PaddedCounters {                 // each counter on its own cache line
    alignas(64) std::int64_t a = 0;
    alignas(64) std::int64_t b = 0;
};

void alignment()
{
    std::cout << "\n=== Alignment ===\n";
    std::cout << "  alignof(char)    = " << alignof(char) << '\n';
    std::cout << "  alignof(double)  = " << alignof(double) << '\n';
    std::cout << "  alignof(Widget)  = " << alignof(Widget) << '\n';
    std::cout << "  sizeof(SharedCounters) = " << sizeof(SharedCounters) << '\n';   // 16
    std::cout << "  sizeof(PaddedCounters) = " << sizeof(PaddedCounters) << '\n';   // 128

    auto padded = std::make_unique<PaddedCounters>();   // C++17: heap allocation honours alignas(64)
    auto address = reinterpret_cast<std::uintptr_t>(padded.get());
    std::cout << "  heap PaddedCounters is 64-byte aligned? " << (address % 64 == 0) << '\n';

#if defined(__cpp_lib_hardware_interference_size)
    std::cout << "  hardware_destructive_interference_size = "
              << std::hardware_destructive_interference_size << '\n';
#else
    std::cout << "  hardware_destructive_interference_size not provided by this library\n";
#endif
}

int main()
{
    placement_new();

    std::cout << "\n=== A Container Built on Raw Storage ===\n";
    {
        // Each push_back copies a temporary Widget into the storage. The
        // copy constructor is the compiler-generated one (it prints
        // nothing), so you see each temporary constructed and destroyed.
        StaticVector<Widget, 3> widgets;
        widgets.push_back(Widget{"first"});
        widgets.push_back(Widget{"second"});
        std::cout << "  size " << widgets.size() << ", widgets[1] = " << widgets[1].name() << '\n';
        std::cout << "  leaving scope:\n";
    }

    alignment();
}
