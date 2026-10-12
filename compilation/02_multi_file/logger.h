// ============================================================
// logger.h: A Small Class Split into Header and Source
// ============================================================
// The class definition (members and declarations) lives here;
// the member function bodies live in logger.cpp. Changing a body
// recompiles one file. Changing this header recompiles every
// file that includes it, directly or indirectly.
// ============================================================

#ifndef CPPZH_LOGGER_H
#define CPPZH_LOGGER_H

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

class Logger {
public:
    explicit Logger(std::string prefix);

    void log(std::string_view message);
    std::size_t count() const;

private:
    std::string prefix_;
    std::vector<std::string> lines_;
};

#endif  // CPPZH_LOGGER_H
