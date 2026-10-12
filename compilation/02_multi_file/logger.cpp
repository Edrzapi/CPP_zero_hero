// ============================================================
// logger.cpp: Member Function Definitions for Logger
// ============================================================
// Out-of-class definitions use the qualified name Logger::log.
// ============================================================

#include "logger.h"

#include <iostream>
#include <utility>

Logger::Logger(std::string prefix) : prefix_{std::move(prefix)} {}

void Logger::log(std::string_view message)
{
    lines_.emplace_back(message);
    std::cout << '[' << prefix_ << "] " << message << '\n';
}

std::size_t Logger::count() const { return lines_.size(); }
