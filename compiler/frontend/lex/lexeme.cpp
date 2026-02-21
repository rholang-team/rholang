#include "compiler/frontend/lex/lexeme.hpp"

namespace frontend::lex {
std::string_view Lexemes::getInput() const {
    return std::string_view{input};
}

std::string_view Lexemes::getLiteral(Span span) const {
    return std::string_view{input}.substr(span.begin, span.length());
}

bool Lexemes::isEof() const {
    return offset >= lexemes.size();
}

Lexeme Lexemes::peek() const {
    return peek(0);
}

Lexeme Lexemes::peek(size_t n) const {
    if (offset + n >= lexemes.size()) {
        return Lexeme{.token = Token::Eof, .span = eofSpan};
    }

    return lexemes[offset + n];
}

Lexeme Lexemes::next() {
    if (offset >= lexemes.size()) {
        return Lexeme{.token = Token::Eof, .span = eofSpan};
    }

    return lexemes[offset++];
}
}  // namespace frontend::lex
