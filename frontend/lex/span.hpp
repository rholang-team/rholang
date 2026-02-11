#pragma once

#include <cstddef>
#include <utility>

namespace frontend::lex {
struct Span {
    size_t begin;
    size_t end;

    Span() : Span(0, 0) {}
    Span(size_t begin, size_t end) : begin{begin}, end{end} {}
    explicit Span(size_t offset) : begin{offset}, end{offset} {}

    size_t length() const {
        return end - begin + 1;
    }
};

template <typename T>
struct WithSpan {
    T value;
    Span span;

    template <typename U>
        requires std::constructible_from<T, U> || std::convertible_to<U, T>
    WithSpan(U&& value, Span span) : value{std::forward<U>(value)}, span{span} {}

    template <typename U>
        requires std::constructible_from<T, U> || std::convertible_to<U, T>
    WithSpan(WithSpan<U>& that) : value{std::forward<U>(that.value)}, span{that.span} {}
};

template <typename T>
WithSpan(T, Span) -> WithSpan<T>;
}  // namespace frontend::lex
