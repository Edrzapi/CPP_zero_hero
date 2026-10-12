// ============================================================
// The Rule of Zero, Three and Five
// ============================================================
// The compiler can generate six special member functions:
//
//   default constructor     T()
//   destructor              ~T()
//   copy constructor        T(const T&)
//   copy assignment         T& operator=(const T&)
//   move constructor        T(T&&)
//   move assignment         T& operator=(T&&)
//
// Rule of Three (C++98): if you write a destructor, copy
//   constructor or copy assignment, you almost certainly need all
//   three, because the class owns a resource.
// Rule of Five (C++11): the same, plus the two move operations.
// Rule of Zero: better still, write NONE of them. Hold resources
//   in members that already manage themselves (std::string,
//   std::vector, std::unique_ptr) and the generated versions are
//   correct automatically.
//
// Build and run:
//   cmake --build build --target memory_04_rule_of_zero_three_five
//   ./build/bin/memory_04_rule_of_zero_three_five
// ============================================================

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// ------------------------------------------------------------
// Why the Defaults Break for Owning Raw Pointers
// ------------------------------------------------------------
// The generated copy constructor copies each member. For a raw
// pointer that copies the ADDRESS (a shallow copy), so two objects
// point at one buffer and both destructors delete it: a double
// free. The class below does it properly.

// ------------------------------------------------------------
// Rule of Five: a Hand-Written Owning Buffer
// ------------------------------------------------------------

class Buffer {
public:
    explicit Buffer(std::size_t size) : size_{size}, data_{new int[size]{}}
    {
        std::cout << "  Buffer(" << size_ << ") allocate\n";
    }

    // 1. Destructor: release what we own.
    ~Buffer()
    {
        if (data_ != nullptr) {
            std::cout << "  ~Buffer free " << size_ << " ints\n";
        }
        delete[] data_;   // delete[] on nullptr is a safe no-op
    }

    // 2. Copy constructor: deep copy into a new allocation.
    Buffer(const Buffer& other) : size_{other.size_}, data_{new int[other.size_]}
    {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "  Buffer copy-construct (deep copy)\n";
    }

    // 3. Copy assignment: copy-and-swap gives the strong exception
    //    guarantee and handles self-assignment for free.
    Buffer& operator=(const Buffer& other)
    {
        std::cout << "  Buffer copy-assign\n";
        Buffer temp{other};   // may throw: *this untouched if it does
        swap(temp);           // cannot throw
        return *this;
    }                         // temp destroys our old buffer

    // 4. Move constructor: steal the pointer, leave the source empty.
    Buffer(Buffer&& other) noexcept
        : size_{std::exchange(other.size_, 0)}, data_{std::exchange(other.data_, nullptr)}
    {
        std::cout << "  Buffer move-construct (pointer stolen)\n";
    }

    // 5. Move assignment.
    Buffer& operator=(Buffer&& other) noexcept
    {
        std::cout << "  Buffer move-assign\n";
        Buffer temp{std::move(other)};
        swap(temp);
        return *this;
    }

    void swap(Buffer& other) noexcept
    {
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

    int& operator[](std::size_t i) { return data_[i]; }
    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    int* data_;
};

void rule_of_five()
{
    std::cout << "=== Rule of Five ===\n";

    Buffer a{3};
    a[0] = 42;

    Buffer b = a;              // copy constructor
    b[0] = 7;
    std::cout << "  a[0] = " << a[0] << ", b[0] = " << b[0] << " (independent copies)\n";

    Buffer c = std::move(a);   // move constructor: a is now empty
    std::cout << "  c[0] = " << c[0] << ", a.size() = " << a.size() << '\n';

    b = c;                     // copy assignment
    c = Buffer{2};             // move assignment from a temporary
    std::cout << "  leaving scope\n";
}

// ------------------------------------------------------------
// Rule of Zero: the Same Behaviour, No Special Members
// ------------------------------------------------------------
// std::vector already copies deeply and moves cheaply, so this
// class needs none of the five. This is what most classes
// should look like.

class SimpleBuffer {
public:
    explicit SimpleBuffer(std::size_t size) : data_(size) {}
    int& operator[](std::size_t i) { return data_[i]; }
    std::size_t size() const { return data_.size(); }

private:
    std::vector<int> data_;
};

// A move-only rule-of-zero class: unique_ptr deletes the copy operations for us.
class Document {
public:
    explicit Document(std::string title) : title_{std::make_unique<std::string>(std::move(title))} {}
    const std::string& title() const { return *title_; }

private:
    std::unique_ptr<std::string> title_;
};

void rule_of_zero()
{
    std::cout << "\n=== Rule of Zero ===\n";

    SimpleBuffer a{3};
    a[0] = 42;
    SimpleBuffer b = a;              // deep copy, generated by the compiler
    b[0] = 7;
    std::cout << "  a[0] = " << a[0] << ", b[0] = " << b[0] << '\n';

    Document doc{"Report"};
    Document moved = std::move(doc); // generated move
    // Document copy = moved;        // error: copy is implicitly deleted
    std::cout << "  moved document: " << moved.title() << '\n';
}

// ------------------------------------------------------------
// = default and = delete
// ------------------------------------------------------------
// Declaring ANY of copy constructor, copy assignment or destructor
// stops the compiler generating the move operations (copies are
// used instead, silently). Declaring a move operation deletes the
// copy operations. Be explicit when in doubt:

class Explicit {
public:
    Explicit() = default;
    ~Explicit() = default;                           // e.g. if you need it virtual
    Explicit(const Explicit&) = default;
    Explicit& operator=(const Explicit&) = default;
    Explicit(Explicit&&) noexcept = default;         // without these, the destructor
    Explicit& operator=(Explicit&&) noexcept = default; // above would disable moves
};

class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;            // copies forbidden
    NonCopyable& operator=(const NonCopyable&) = delete;
};

int main()
{
    rule_of_five();
    rule_of_zero();

    Explicit e1;
    Explicit e2 = std::move(e1);
    NonCopyable n;
    // NonCopyable n2 = n;   // error: use of deleted function
    std::cout << "\n=== = default / = delete compiled as expected ===\n";
    (void)e2;
    (void)n;
}
