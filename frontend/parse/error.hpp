#pragma once

#include <format>
#include <string>

#include "frontend/error.hpp"
#include "frontend/lex/span.hpp"

namespace frontend::parse {
template <typename T>
std::string formatExpectedList(const T& e) {
    return std::format("{}", e);
}

template <typename T, typename U>
std::string formatExpectedList(const T& e1, const U& e2) {
    return std::format("{} or {}", e1, e2);
}

template <typename T, typename... Ts>
std::string formatExpectedList(const T& e, const Ts&... es) {
    return std::format("{}, {}", e, formatExpectedList(es...));
}

template <typename... Exp, typename Act>
Error error(std::string_view input, lex::Span span, const Act& actual, const Exp&... expected) {
    return Error{input,
                 span,
                 std::format("expected {}, but got {}", formatExpectedList(expected...), actual)};
}
}  // namespace frontend::parse
