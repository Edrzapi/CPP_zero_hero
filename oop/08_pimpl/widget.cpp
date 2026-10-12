// ============================================================
// widget.cpp: The Hidden Implementation
// ============================================================
// Everything private lives here. Change Impl (add a member, swap
// std::vector for std::set) and only this file recompiles.
// ============================================================

#include "widget.h"

#include <map>
#include <sstream>
#include <utility>
#include <vector>

struct Widget::Impl {
    std::string name;
    std::vector<std::string> tags;
    std::map<std::string, int> usage;   // a "heavy" detail users never see

    std::string describe() const
    {
        std::ostringstream out;
        out << name << " [";
        for (std::size_t i = 0; i < tags.size(); ++i) {
            out << (i ? ", " : "") << tags[i];
        }
        out << "] (" << usage.size() << " distinct tags)";
        return out.str();
    }
};

Widget::Widget(std::string name) : impl_{std::make_unique<Impl>()}
{
    impl_->name = std::move(name);
}

// Defined here, where Impl is complete. "= default" is enough.
Widget::~Widget() = default;
Widget::Widget(Widget&&) noexcept = default;
Widget& Widget::operator=(Widget&&) noexcept = default;

Widget::Widget(const Widget& other) : impl_{std::make_unique<Impl>(*other.impl_)} {}

Widget& Widget::operator=(const Widget& other)
{
    if (this != &other) {
        impl_ = std::make_unique<Impl>(*other.impl_);
    }
    return *this;
}

void Widget::add_tag(const std::string& tag)
{
    impl_->tags.push_back(tag);
    ++impl_->usage[tag];
}

std::string Widget::describe() const { return impl_->describe(); }
