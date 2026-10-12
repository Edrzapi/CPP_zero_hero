// ============================================================
// Associative Containers: map, set and Their Unordered Twins
// ============================================================
//   std::map / std::set                  sorted, balanced tree, O(log n)
//   std::unordered_map / unordered_set   hash table, O(1) average
//   multi- versions                      allow duplicate keys
//
// Java: TreeMap / HashMap.  C#: SortedDictionary / Dictionary.
// Python: dict is a hash map that keeps insertion order; no
// standard C++ container does that.
//
// Build and run:
//   cmake --build build --target collections_03_associative
//   ./build/bin/collections_03_associative
// ============================================================

#include <cstddef>
#include <functional>   // std::hash
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

// ------------------------------------------------------------
// std::map
// ------------------------------------------------------------
// The operator[] trap: m[key] INSERTS a default value if the key
// is missing. Reading with [] on a map you only meant to query
// silently grows it (and does not compile on a const map).
// Use find(), contains() (C++20) or at() to look up safely.

void map_demo()
{
    std::cout << "=== std::map ===\n";

    std::map<std::string, int> stock{{"pears", 4}, {"apples", 10}};
    stock["plums"] = 7;               // insert or overwrite
    stock["apples"] += 5;

    for (const auto& [fruit, count] : stock) {   // iterates in sorted key order
        std::cout << "  " << fruit << ": " << count << '\n';
    }

    std::cout << "  contains(\"kiwi\") = " << std::boolalpha << stock.contains("kiwi") << '\n';
    int kiwis = stock["kiwi"];        // TRAP: inserts kiwi with value 0
    std::cout << "  after reading stock[\"kiwi\"] (" << kiwis << "), size = " << stock.size() << '\n';

    if (auto it = stock.find("pears"); it != stock.end()) {
        std::cout << "  find(\"pears\") -> " << it->second << '\n';
    }

    // Insertion helpers:
    auto [pos, inserted] = stock.insert({"apples", 1});   // does NOT overwrite
    std::cout << "  insert existing apples: inserted = " << inserted << ", value " << pos->second << '\n';
    stock.insert_or_assign("apples", 1);                  // always writes (C++17)
    stock.try_emplace("figs", 3);                         // constructs only if absent (C++17)
    std::cout << "  apples now " << stock.at("apples") << ", figs " << stock.at("figs") << '\n';

    // Sorted order allows range queries.
    auto from = stock.lower_bound("b");
    auto to = stock.upper_bound("l");
    std::cout << "  keys between b and l:";
    for (auto it = from; it != to; ++it) std::cout << ' ' << it->first;
    std::cout << '\n';
}

// ------------------------------------------------------------
// std::set and std::multiset
// ------------------------------------------------------------

void set_demo()
{
    std::cout << "\n=== std::set and std::multiset ===\n";

    std::set<int> unique{5, 1, 5, 3, 1};
    std::cout << "  set:";
    for (int x : unique) std::cout << ' ' << x;          // 1 3 5: sorted, no duplicates
    std::cout << '\n';

    std::multiset<int> counts{5, 1, 5, 3, 1};
    std::cout << "  multiset count(5) = " << counts.count(5) << '\n';

    // A custom ordering: longest strings first, then alphabetical.
    auto by_length_desc = [](const std::string& a, const std::string& b) {
        return a.size() != b.size() ? a.size() > b.size() : a < b;
    };
    std::set<std::string, decltype(by_length_desc)> words({"fig", "banana", "kiwi", "apple"}, by_length_desc);
    std::cout << "  custom order:";
    for (const auto& w : words) std::cout << ' ' << w;
    std::cout << '\n';
}

// ------------------------------------------------------------
// Unordered Containers and Custom Hashes
// ------------------------------------------------------------
// Iteration order is unspecified and can change after a rehash.
// A key type needs == and a std::hash specialisation (or a hash
// functor passed as a template argument).

struct Postcode {
    std::string outward;   // e.g. "SW1A"
    std::string inward;    // e.g. "1AA"
    bool operator==(const Postcode&) const = default;
};

struct PostcodeHash {
    std::size_t operator()(const Postcode& p) const noexcept
    {
        // Combine the two member hashes. (Boost's hash_combine is the
        // usual recipe; this simple version is fine for teaching.)
        std::size_t h1 = std::hash<std::string>{}(p.outward);
        std::size_t h2 = std::hash<std::string>{}(p.inward);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

void unordered_demo()
{
    std::cout << "\n=== Unordered Containers ===\n";

    std::unordered_map<std::string, int> word_count;
    for (const char* w : {"to", "be", "or", "not", "to", "be"}) {
        ++word_count[w];   // here the insert-on-read behaviour is exactly what we want
    }
    std::cout << "  'to' appears " << word_count["to"] << " times; "
              << word_count.size() << " distinct words\n";
    std::cout << "  buckets: " << word_count.bucket_count()
              << ", load factor: " << word_count.load_factor() << '\n';

    std::unordered_set<Postcode, PostcodeHash> delivered{{"SW1A", "1AA"}, {"EC1A", "1BB"}};
    std::cout << "  delivered to SW1A 1AA? " << delivered.contains(Postcode{"SW1A", "1AA"}) << '\n';
}

int main()
{
    map_demo();
    set_demo();
    unordered_demo();
}
