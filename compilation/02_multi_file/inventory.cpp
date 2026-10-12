// ============================================================
// inventory.cpp: Uses the Full Logger Definition
// ============================================================
// Calling logger_.log() needs the complete class, so the .cpp
// includes logger.h. The dependency is hidden from every file
// that only includes inventory.h.
// ============================================================

#include "inventory.h"
#include "logger.h"

#include <string>

Inventory::Inventory(Logger& logger) : logger_{logger} {}

void Inventory::add(const std::string& item, int quantity)
{
    stock_[item] += quantity;
    logger_.log("added " + std::to_string(quantity) + " x " + item);
}

int Inventory::quantity_of(const std::string& item) const
{
    auto it = stock_.find(item);
    return it == stock_.end() ? 0 : it->second;
}
