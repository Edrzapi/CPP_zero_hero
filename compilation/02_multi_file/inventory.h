// ============================================================
// inventory.h: Forward Declarations
// ============================================================
// Inventory only stores a REFERENCE to a Logger and only uses it
// inside inventory.cpp. A reference or pointer does not need the
// full class definition, so instead of #include "logger.h" this
// header says "class Logger;" (a forward declaration).
//
// Benefit: files that include inventory.h no longer depend on
// logger.h. Editing logger.h does not rebuild them. In large
// projects this is one of the cheapest build-time wins there is.
//
// A forward-declared (incomplete) type can be used for:
//   pointers and references, function parameters and return
//   types in declarations, friend declarations.
// It can NOT be used for:
//   members held by value, base classes, sizeof, calling members.
// ============================================================

#ifndef CPPZH_INVENTORY_H
#define CPPZH_INVENTORY_H

#include <map>
#include <string>

class Logger;   // forward declaration: no #include "logger.h" needed here

class Inventory {
public:
    explicit Inventory(Logger& logger);

    void add(const std::string& item, int quantity);
    int quantity_of(const std::string& item) const;

private:
    Logger& logger_;                 // fine with an incomplete type
    std::map<std::string, int> stock_;
    // Logger by_value_;             // error here: Logger is incomplete
};

#endif  // CPPZH_INVENTORY_H
