#pragma once

#include <string>
#include <vector>

#include "frontend/lex/span.hpp"
#include "frontend/lex/token.hpp"

namespace frontend::lex {
struct Lexeme {
    Token token;
    Span span;
};

class Lexemes {
    std::string input;
    std::vector<Lexeme> lexemes;
    Span eofSpan;
    size_t offset = 0;

public:
    Lexemes(std::string input, std::vector<Lexeme> lexemes, Span eofSpan)
        : input{std::move(input)},
          lexemes{std::move(lexemes)},
          eofSpan{eofSpan} {}

    std::string_view getInput() const;
    std::string_view getLiteral(Span span) const;

    bool isEof() const;

    /// Peek next lexeme
    Lexeme peek() const;

    /// Peek n'th lexeme
    Lexeme peek(size_t n) const;

    /// Extract a lexeme
    Lexeme next();
};
}  // namespace frontend::lex
