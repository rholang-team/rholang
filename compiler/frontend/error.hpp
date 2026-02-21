#pragma once

#include <string>

#include "compiler/frontend/lex/span.hpp"

namespace frontend {
class Error final : public std::exception {
    std::string_view input;
    lex::Span span;
    std::string msg;

public:
    template <typename S>
        requires std::convertible_to<std::string, S> ||
                     std::constructible_from<std::string, S>
    Error(std::string_view input, lex::Span span, S&& msg)
        : input{input}, span{span}, msg{std::move(msg)} {}

    std::string pretty() const;
};
}  // namespace frontend
