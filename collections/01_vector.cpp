// ============================================================
// std::vector: The Default Container
// ============================================================
// A dynamic array: contiguous memory, O(1) indexing, amortised
// O(1) push_back. Because the elements sit next to each other,
// it is cache-friendly and usually beats "cleverer" containers
// even where textbook complexity says otherwise. Use vector
// unless you have measured a reason not to.
// Equivalent to Java's ArrayList, C#'s List<T>, Python's list.
//
// Build and run:
//   cmake --build build --target collections_01_vector
//   ./build/bin/collections_01_vector
// ============================================================

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

template <typename T>
void print(const std::string& label, const std::vector<T>& v)
{
    std::cout << "  " << label << ": [";
    for (std::size_t i = 0; i < v.size(); ++i) {
        std::cout << (i ? ", " : "") << v[i];
    }
    std::cout << "] size " << v.size() << '\n';
}

// ------------------------------------------------------------
// Creating and Adding
// ------------------------------------------------------------
// push_back copies or moves an existing object in.
// emplace_back constructs the element in place from arguments.

void basics()
{
    std::cout << "=== Creating and Adding ===\n";

    std::vector<int> empty;
    std::vector<int> zeros(3);             // 3 value-initialised ints: 0 0 0
    std::vector<int> listed{4, 8, 15};     // initializer_list
    std::vector<std::string> names(2, "x"); // 2 copies of "x"

    listed.push_back(16);
    listed.insert(listed.begin(), 1);      // O(n): shifts everything after it
    names.emplace_back(3, 'z');            // constructs std::string(3, 'z') = "zzz" in place

    print("empty", empty);
    print("zeros", zeros);
    print("listed", listed);
    print("names", names);
}

// ------------------------------------------------------------
// Access
// ------------------------------------------------------------
// v[i]      no bounds check: out of range is undefined behaviour
// v.at(i)   bounds checked: throws std::out_of_range
// front(), back(), data() (pointer for C APIs)

void access()
{
    std::cout << "\n=== Access ===\n";
    std::vector<int> v{10, 20, 30};
    std::cout << "  v[1] = " << v[1] << ", front = " << v.front() << ", back = " << v.back() << '\n';
    try {
        std::cout << v.at(10);
    } catch (const std::out_of_range&) {
        std::cout << "  v.at(10) threw std::out_of_range\n";
    }
}

// ------------------------------------------------------------
// Size vs Capacity
// ------------------------------------------------------------
// size():     elements in use
// capacity(): elements the current allocation can hold
// When size would exceed capacity, vector allocates a bigger block
// (typically 1.5x or 2x), moves the elements over and frees the old
// one. reserve() allocates up front when you know the final size.

void capacity()
{
    std::cout << "\n=== Size vs Capacity ===\n";

    std::vector<int> grows;
    std::size_t last_capacity = grows.capacity();
    for (int i = 0; i < 20; ++i) {
        grows.push_back(i);
        if (grows.capacity() != last_capacity) {
            std::cout << "  size " << grows.size() << " -> capacity " << grows.capacity() << '\n';
            last_capacity = grows.capacity();
        }
    }
    // MSVC grows by 1.5x (1 2 3 4 6 9 13 19 28), GCC and Clang by 2x (1 2 4 8 16 32).

    std::vector<int> reserved;
    reserved.reserve(100);
    std::cout << "  after reserve(100): size " << reserved.size() << ", capacity " << reserved.capacity() << '\n';

    grows.clear();                 // size 0, capacity unchanged
    grows.shrink_to_fit();         // non-binding request to release memory
    std::cout << "  after clear + shrink_to_fit: capacity " << grows.capacity() << '\n';
}

// ------------------------------------------------------------
// Iterator Invalidation
// ------------------------------------------------------------
// A reallocation moves every element, so pointers, references and
// iterators into the vector become dangling. insert/erase also
// invalidate everything after the change point.
//
//   int& first = v[0];
//   v.push_back(99);      // may reallocate
//   first = 1;            // undefined behaviour if it did
//
// Hold indexes instead, or reserve() first, or re-fetch after.

void invalidation()
{
    std::cout << "\n=== Invalidation (Safe Version) ===\n";
    std::vector<int> v{1, 2, 3};
    std::size_t index_of_first = 0;     // indexes survive reallocation
    v.push_back(4);
    v[index_of_first] = 100;
    print("v", v);
}

// ------------------------------------------------------------
// Erasing
// ------------------------------------------------------------
// C++20 std::erase / std::erase_if replace the old erase-remove
// idiom: v.erase(std::remove(v.begin(), v.end(), x), v.end());

void erasing()
{
    std::cout << "\n=== Erasing ===\n";
    std::vector<int> v{1, 2, 3, 2, 4, 2};
    std::erase(v, 2);
    print("after erase(v, 2)", v);
    std::erase_if(v, [](int x) { return x > 3; });
    print("after erase_if > 3", v);
    v.pop_back();
    print("after pop_back", v);
}

// ------------------------------------------------------------
// vector<bool>
// ------------------------------------------------------------
// A historical oddity: vector<bool> is specialised to pack bits,
// so v[0] returns a proxy object, not a bool&, and &v[0] is not a
// bool*. Generic code that works for every other vector can break
// on it. Use std::vector<char>, std::bitset or std::deque<bool>
// when that matters.

int main()
{
    basics();
    access();
    capacity();
    invalidation();
    erasing();
}
