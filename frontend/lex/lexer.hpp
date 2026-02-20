#pragma once

#include <optional>
#include <string>

#include "frontend/lex/lexeme.hpp"

namespace frontend::lex {
class Lexer {
    std::string input;
    size_t offset = 0;

    std::optional<char> peekChar() const;
    std::string_view peekChars(size_t n) const;

    std::optional<char> nextChar();

    void rollback();

    std::optional<Lexeme> nextLexeme();

    template <typename F>
        requires std::predicate<F, char>
    void skipWhile(F&& pred) {
        while (offset < input.size() && pred(input[offset])) {
            offset++;
        }
    }

    template <typename F>
        requires std::predicate<F, char>
    Span getWhile(F&& pred) {
        size_t start = offset;

        while (offset < input.size() && pred(input[offset])) {
            offset++;
        }

        if (offset > start) {
            return Span{start, offset - 1};
        } else {
            return Span{start, start};
        }
    }

public:
    template <typename S>
        requires std::convertible_to<std::string, S> ||
                 std::constructible_from<std::string, S>
    explicit Lexer(S&& s) : input{std::forward<S>(s)} {}

    Lexemes lex();
};
}  // namespace frontend::lex
