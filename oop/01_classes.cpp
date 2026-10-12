// ============================================================
// Classes: Members, Access, const, static and friend
// ============================================================
// The building blocks before inheritance. The main surprises
// for Java and C# developers: objects are values (not references),
// class and struct differ only in default access, and const is
// part of a member function's signature.
//
// Build and run:
//   cmake --build build --target oop_01_classes
//   ./build/bin/oop_01_classes
// ============================================================

#include <iostream>
#include <string>
#include <utility>

// ------------------------------------------------------------
// class vs struct
// ------------------------------------------------------------
// The ONLY difference: members and bases of a struct are public
// by default, of a class private by default. Convention: struct
// for plain data with no invariants, class when there is
// something to protect.

struct Point {          // public by default
    double x = 0;
    double y = 0;
};

// ------------------------------------------------------------
// Encapsulation and Invariants
// ------------------------------------------------------------
// A class INVARIANT is a rule that must always hold, such as
// "balance is never negative". Private data plus public member
// functions that check the rule keep it true.
// Trailing underscores on members are one common naming
// convention (others: m_balance, or no decoration).

class BankAccount {
public:
    BankAccount(std::string owner, double opening_balance)
        : owner_{std::move(owner)}, balance_{opening_balance > 0 ? opening_balance : 0}
    {
        ++accounts_open_;
    }

    ~BankAccount() { --accounts_open_; }

    // Copying a bank account makes little sense: forbid it.
    BankAccount(const BankAccount&) = delete;
    BankAccount& operator=(const BankAccount&) = delete;

    // ---- const member functions ----
    // "const" after the parameter list: this function does not
    // modify the object. Only const members can be called on a
    // const object or through a const reference.
    double balance() const { return balance_; }
    const std::string& owner() const { return owner_; }

    // ---- non-const member functions ----
    bool withdraw(double amount)
    {
        ++operations_;   // a mutable counter, see "mutable" below
        if (amount <= 0 || amount > balance_) {
            return false;   // invariant protected
        }
        balance_ -= amount;
        return true;
    }

    void deposit(double amount)
    {
        ++operations_;
        if (amount > 0) balance_ += amount;
    }

    // "this" is a pointer to the current object. Returning *this
    // by reference allows chaining: account.deposit(1).deposit(2)
    BankAccount& add_interest(double rate)
    {
        this->balance_ += balance_ * rate;   // this-> is optional here
        return *this;
    }

    // ---- mutable ----
    // A mutable member may change even in a const function.
    // Use only for things that do not affect the observable state:
    // caches, counters, mutexes.
    int operations() const { return operations_; }

    // ---- static members ----
    // Shared by all objects of the class, like Java/C# static.
    static int accounts_open() { return accounts_open_; }

    // ---- friend ----
    // A friend function is not a member but may access private
    // members. Used mainly for operators (see 09_operator_overloading).
    friend void audit(const BankAccount& account);

private:
    std::string owner_;
    double balance_;
    mutable int operations_ = 0;
    static inline int accounts_open_ = 0;   // C++17: defined right here, no .cpp needed
};

void audit(const BankAccount& account)
{
    std::cout << "  audit: " << account.owner_ << " holds " << account.balance_
              << " after " << account.operations_ << " operations\n";
}

void print_summary(const BankAccount& account)   // const&: only const members are callable
{
    std::cout << "  " << account.owner() << ": " << account.balance() << '\n';
    // account.deposit(10);   // error: deposit is not const
}

// ------------------------------------------------------------
// Objects Are Values
// ------------------------------------------------------------
// In Java, "Point b = a;" copies a reference: both names see one
// object. In C++ it copies the OBJECT. To share, use a reference
// or pointer explicitly.

void objects_are_values()
{
    std::cout << "\n=== Objects Are Values ===\n";
    Point a{1, 2};
    Point b = a;     // an independent copy
    Point& r = a;    // a reference: another name for a
    b.x = 100;
    r.y = 200;
    std::cout << "  a = (" << a.x << ", " << a.y << "), b = (" << b.x << ", " << b.y << ")\n";
}

int main()
{
    std::cout << "=== Encapsulation ===\n";
    BankAccount account{"Ada", 100};
    account.deposit(50);
    std::cout << "  withdraw 500 ok? " << std::boolalpha << account.withdraw(500) << '\n';
    account.add_interest(0.10).add_interest(0.10);   // chained through *this
    print_summary(account);
    audit(account);

    std::cout << "\n=== static Members ===\n";
    {
        BankAccount second{"Alan", 10};
        std::cout << "  accounts open: " << BankAccount::accounts_open() << '\n';   // 2
    }
    std::cout << "  accounts open: " << BankAccount::accounts_open() << '\n';       // 1

    objects_are_values();
}
