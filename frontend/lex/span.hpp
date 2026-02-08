#pragma once

#include <cstddef>

namespace frontend::lex {
struct Span {
    size_t begin;
    size_t end;

    Span(size_t begin, size_t end) : begin{begin}, end{end} {}
    explicit Span(size_t offset) : begin{offset}, end{offset} {}

    size_t length() const {
        return end - begin + 1;
    }
};
}  // namespace frontend::lex
