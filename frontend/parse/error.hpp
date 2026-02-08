#pragma once

#include <exception>
#include <format>
#include <string>

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

class ParseError final : public std::exception {
    std::string_view input;
    lex::Span span;
    std::string msg;

    ParseError(std::string_view input, lex::Span span, std::string msg)
        : input{input}, span{span}, msg{std::move(msg)} {}

public:
    template <typename S>
        requires std::convertible_to<std::string, S> || std::constructible_from<std::string, S>
    static ParseError customMessage(std::string_view input, lex::Span span, S&& msg) {
        return ParseError(input, span, std::string{std::forward<S>(msg)});
    }

    template <typename... Exp, typename Act>
    ParseError(std::string_view input, lex::Span span, const Act& actual, const Exp&... expected)
        : input{input},
          span{span},
          msg{std::format("expected {}, but got {}", formatExpectedList(expected...), actual)} {}

    std::string pretty() const;
};
}  // namespace frontend::parse
