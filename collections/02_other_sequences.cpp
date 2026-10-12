// ============================================================
// Other Sequence Containers and Container Adaptors
// ============================================================
// std::array, std::deque, std::list and std::forward_list, plus
// the adaptors std::stack, std::queue and std::priority_queue that
// wrap another container with a restricted interface.
//
// Build and run:
//   cmake --build build --target collections_02_other_sequences
//   ./build/bin/collections_02_other_sequences
// ============================================================

#include <array>
#include <deque>
#include <forward_list>
#include <functional>   // std::greater
#include <iostream>
#include <iterator>     // std::next
#include <list>
#include <queue>
#include <stack>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Choosing a Sequence Container
// ------------------------------------------------------------
//                  index  push_back  push_front  insert middle  contiguous
//   array          O(1)   fixed      fixed       no             yes
//   vector         O(1)   O(1)*      O(n)        O(n)           yes
//   deque          O(1)   O(1)       O(1)        O(n)           no (blocks)
//   list           O(n)   O(1)       O(1)        O(1)**         no (nodes)
//   forward_list   O(n)   no         O(1)        O(1)**         no (nodes)
//   * amortised   ** once you hold an iterator to the position
//
// Linked lists look good on paper but every node is a separate
// heap allocation and traversal misses the cache constantly. In
// practice a vector often wins even for middle insertions up to
// thousands of elements. Measure before choosing list.

void deque_demo()
{
    std::cout << "=== std::deque ===\n";
    std::deque<int> d{2, 3};
    d.push_front(1);
    d.push_back(4);
    std::cout << "  ";
    for (int x : d) std::cout << x << ' ';
    std::cout << "| d[2] = " << d[2] << '\n';
    // Unlike vector, push_front/push_back never move existing elements,
    // so references to elements stay valid (iterators do not).
}

void list_demo()
{
    std::cout << "\n=== std::list and std::forward_list ===\n";

    std::list<std::string> tasks{"write", "test", "ship"};
    auto it = std::next(tasks.begin());       // points at "test"
    tasks.insert(it, "review");               // O(1), "it" still valid
    tasks.remove("ship");
    tasks.sort();                             // member sort: std::sort needs random access
    std::cout << "  list: ";
    for (const auto& t : tasks) std::cout << t << ' ';
    std::cout << '\n';

    // splice moves nodes between lists without copying any element.
    std::list<std::string> later{"deploy"};
    tasks.splice(tasks.end(), later);
    std::cout << "  after splice: " << tasks.size() << " tasks, later has " << later.size() << '\n';

    std::forward_list<int> singly{3, 2, 1};   // one pointer per node, no size()
    singly.push_front(4);
    std::cout << "  forward_list: ";
    for (int x : singly) std::cout << x << ' ';
    std::cout << '\n';
}

// ------------------------------------------------------------
// Container Adaptors
// ------------------------------------------------------------
// stack:          LIFO, top/push/pop (default container: deque)
// queue:          FIFO, front/back/push/pop (default: deque)
// priority_queue: largest element on top (default: vector + heap)
// Note: pop() returns void. Read top()/front() first, then pop().
// (Returning the value from pop could throw after the element was
// already removed, losing it.)

void adaptors()
{
    std::cout << "\n=== stack, queue, priority_queue ===\n";

    std::stack<std::string> undo;
    undo.push("type a");
    undo.push("type b");
    undo.push("delete");
    std::cout << "  undo order: ";
    while (!undo.empty()) {
        std::cout << undo.top() << " | ";
        undo.pop();
    }
    std::cout << '\n';

    std::queue<int> line;
    for (int customer = 1; customer <= 3; ++customer) line.push(customer);
    std::cout << "  served first: " << line.front() << ", last in line: " << line.back() << '\n';

    std::priority_queue<int> max_heap;
    for (int x : {5, 1, 9, 3}) max_heap.push(x);
    std::cout << "  max-heap pops: ";
    while (!max_heap.empty()) {
        std::cout << max_heap.top() << ' ';
        max_heap.pop();
    }
    std::cout << '\n';

    // A min-heap: supply the container and the comparison.
    std::priority_queue<int, std::vector<int>, std::greater<>> min_heap;
    for (int x : {5, 1, 9, 3}) min_heap.push(x);
    std::cout << "  min-heap top: " << min_heap.top() << '\n';
}

int main()
{
    deque_demo();
    list_demo();
    adaptors();

    std::cout << "\n=== std::array (fixed size, see basics/11_arrays) ===\n";
    std::array<std::string, 3> days{"Mon", "Tue", "Wed"};
    std::cout << "  " << days.size() << " days, last is " << days.back() << '\n';
}
