// ============================================================
// Smart Pointers: unique_ptr, shared_ptr, weak_ptr
// ============================================================
// Smart pointers are RAII for heap memory. They express
// OWNERSHIP in the type:
//
//   std::unique_ptr<T>   exactly one owner; move-only; zero overhead
//   std::shared_ptr<T>   shared ownership; reference counted
//   std::weak_ptr<T>     observes a shared_ptr without owning it
//   T* or T&             non-owning: "I use it, someone else owns it"
//
// Default to unique_ptr. Reach for shared_ptr only when ownership
// is genuinely shared and lifetimes cannot be predicted.
//
// Build and run:
//   cmake --build build --target memory_03_smart_pointers
//   ./build/bin/memory_03_smart_pointers
// ============================================================

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Widget {
public:
    explicit Widget(std::string name) : name_{std::move(name)}
    {
        std::cout << "  + Widget " << name_ << '\n';
    }
    ~Widget() { std::cout << "  - Widget " << name_ << '\n'; }
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// ------------------------------------------------------------
// unique_ptr
// ------------------------------------------------------------
// Same size and speed as a raw pointer. Cannot be copied, only
// moved, so there is never any doubt who will delete the object.
// Create with std::make_unique (C++14).

std::unique_ptr<Widget> make_widget(const std::string& name)
{
    return std::make_unique<Widget>(name);   // ownership passes to the caller
}

void take_ownership(std::unique_ptr<Widget> w)   // by value: the caller must std::move
{
    std::cout << "  take_ownership now owns " << w->name() << '\n';
}   // destroyed here

void just_use(const Widget& w)   // non-owning: pass a reference, not a smart pointer
{
    std::cout << "  just_use sees " << w.name() << '\n';
}

void unique_ptr_demo()
{
    std::cout << "=== unique_ptr ===\n";

    auto a = make_widget("A");
    just_use(*a);

    // auto copy = a;                    // error: unique_ptr cannot be copied
    take_ownership(std::move(a));        // explicit transfer
    std::cout << "  after move, a is " << (a ? "set" : "null") << '\n';

    // Containers of unique_ptr are fine: elements are moved, never copied.
    std::vector<std::unique_ptr<Widget>> widgets;
    widgets.push_back(std::make_unique<Widget>("B"));
    widgets.push_back(std::make_unique<Widget>("C"));
    std::cout << "  vector holds " << widgets.size() << " widgets\n";
    std::cout << "  sizeof(unique_ptr<Widget>) = " << sizeof(std::unique_ptr<Widget>)
              << ", sizeof(Widget*) = " << sizeof(Widget*) << '\n';
}   // B and C destroyed with the vector

// ------------------------------------------------------------
// Custom Deleters
// ------------------------------------------------------------
// unique_ptr can manage any resource with a release function,
// not only memory. A stateless deleter adds no size.

struct FileCloser {
    void operator()(std::FILE* f) const noexcept
    {
        std::fclose(f);
        std::cout << "  FileCloser closed the file\n";
    }
};

void custom_deleter_demo()
{
    std::cout << "\n=== Custom Deleter ===\n";
    std::unique_ptr<std::FILE, FileCloser> file{std::tmpfile()};
    if (file) {
        std::fputs("temporary data", file.get());
        std::cout << "  wrote to a temporary file via unique_ptr\n";
    }
}

// ------------------------------------------------------------
// shared_ptr
// ------------------------------------------------------------
// A control block holds the reference count. Copying increments
// it, destroying decrements it, and the object is deleted when it
// reaches zero. The count is updated atomically, so copies are
// not free. std::make_shared does ONE allocation for the object
// and control block together.

void shared_ptr_demo()
{
    std::cout << "\n=== shared_ptr ===\n";

    auto first = std::make_shared<Widget>("Shared");
    std::cout << "  use_count = " << first.use_count() << '\n';     // 1
    {
        auto second = first;                                       // copy: count goes up
        std::cout << "  use_count = " << first.use_count() << '\n'; // 2
    }
    std::cout << "  use_count = " << first.use_count() << '\n';     // 1
}   // count reaches 0: Widget destroyed

// ------------------------------------------------------------
// weak_ptr and Reference Cycles
// ------------------------------------------------------------
// Two objects holding shared_ptrs to each other keep each other
// alive forever: a leak, even with smart pointers. Break the
// cycle by making one side (usually the "back" pointer) weak.
// lock() turns a weak_ptr into a shared_ptr if the object is
// still alive, or an empty one if it is gone.

struct Child;

struct Parent {
    std::string name;
    std::shared_ptr<Child> child;
    ~Parent() { std::cout << "  - Parent " << name << '\n'; }
};

struct Child {
    std::string name;
    std::weak_ptr<Parent> parent;     // weak: does not keep the parent alive
    ~Child() { std::cout << "  - Child " << name << '\n'; }
};

void weak_ptr_demo()
{
    std::cout << "\n=== weak_ptr Breaks Cycles ===\n";

    auto parent = std::make_shared<Parent>();
    parent->name = "P";
    parent->child = std::make_shared<Child>();
    parent->child->name = "C";
    parent->child->parent = parent;

    if (auto p = parent->child->parent.lock()) {
        std::cout << "  child's parent is " << p->name << '\n';
    }

    std::weak_ptr<Parent> observer = parent;
    parent.reset();   // last owner gone: Parent and Child are destroyed
    std::cout << "  observer expired? " << std::boolalpha << observer.expired() << '\n';
    // If Child::parent were a shared_ptr, neither destructor would ever run.
}

int main()
{
    unique_ptr_demo();
    custom_deleter_demo();
    shared_ptr_demo();
    weak_ptr_demo();
}
