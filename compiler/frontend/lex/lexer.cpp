#include "compiler/frontend/lex/lexer.hpp"

#include <array>
#include <optional>
#include <stdexcept>
#include <vector>

#include "compiler/frontend/lex/lexeme.hpp"
#include "compiler/frontend/lex/span.hpp"
#include "compiler/frontend/lex/token.hpp"

#define SINGLE_CHAR_TOKEN(c, tok)                                \
    {                                                            \
        auto ch = peekChar();                                    \
        if (ch.has_value() && *ch == c) {                        \
            return Lexeme{.token = tok, .span = Span{offset++}}; \
        }                                                        \
    }

#define TWO_CHAR_TOKEN(c1, c2, tok1, tok2)                                     \
    {                                                                          \
        auto ch = peekChar();                                                  \
        if (ch.has_value() && *ch == c1) {                                     \
            size_t offset1 = offset++;                                         \
            ch = peekChar();                                                   \
            if (ch.has_value() && *ch == c2) {                                 \
                return Lexeme{.token = tok2, .span = Span{offset1, offset++}}; \
            }                                                                  \
            return Lexeme{.token = tok1, .span = Span{offset1}};               \
        }                                                                      \
    }

#define STRICTLY_TWO_CHAR_TOKEN(c1, c2, tok)                                  \
    {                                                                         \
        auto ch = peekChar();                                                 \
        if (ch.has_value() && *ch == c1) {                                    \
            auto offset1 = ++offset;                                          \
            ch = peekChar();                                                  \
            if (ch.has_value() && *ch == c2) {                                \
                return Lexeme{.token = tok, .span = Span{offset1, offset++}}; \
            } else                                                            \
                rollback();                                                   \
        }                                                                     \
    }

namespace {
bool isIdentifierStart(char c) {
    return std::isalpha(c) || c == '_';
}

bool isIdentifierChar(char c) {
    return std::isalnum(c) || c == '_';
}
}  // namespace

namespace frontend::lex {
Lexemes Lexer::lex() {
    std::vector<Lexeme> lexemes;

    for (;;) {
        auto lexeme = nextLexeme();
        if (!lexeme.has_value()) {
            break;
        }

        lexemes.push_back(*lexeme);
    }

    return Lexemes{input, std::move(lexemes), Span(input.size() - 1)};
}

std::optional<Lexeme> Lexer::nextLexeme() {
    // skip whitespace and comments
    skipWhile([](char c) { return std::isspace(c); });
    if (peekChars(2) == "//") {
        skipWhile([](char c) { return c != '\n'; });
    }

    SINGLE_CHAR_TOKEN('.', Token::Dot);
    SINGLE_CHAR_TOKEN(',', Token::Comma);
    SINGLE_CHAR_TOKEN(':', Token::Colon);
    SINGLE_CHAR_TOKEN(';', Token::Semicolon);
    SINGLE_CHAR_TOKEN('(', Token::LParen);
    SINGLE_CHAR_TOKEN(')', Token::RParen);
    SINGLE_CHAR_TOKEN('{', Token::LBrace);
    SINGLE_CHAR_TOKEN('}', Token::RBrace);

    TWO_CHAR_TOKEN('=', '=', Token::Assign, Token::Eq);
    TWO_CHAR_TOKEN('!', '=', Token::Bang, Token::Ne);
    TWO_CHAR_TOKEN('<', '=', Token::Lt, Token::Le);
    TWO_CHAR_TOKEN('>', '=', Token::Gt, Token::Ge);
    TWO_CHAR_TOKEN('+', '=', Token::Plus, Token::PlusAssign);
    TWO_CHAR_TOKEN('-', '=', Token::Minus, Token::MinusAssign);
    TWO_CHAR_TOKEN('*', '=', Token::Asterisk, Token::MulAssign);
    TWO_CHAR_TOKEN('/', '=', Token::Slash, Token::DivAssign);

    STRICTLY_TWO_CHAR_TOKEN('&', '&', Token::And);
    STRICTLY_TWO_CHAR_TOKEN('|', '|', Token::Or);

    auto ch = peekChar();
    if (!ch.has_value()) {
        return std::nullopt;
    }

    if (std::isdigit(*ch)) {
        Span span = getWhile([](char c) { return std::isdigit(c); });
        return Lexeme{.token = Token::Num, .span = span};
    }

    if (isIdentifierStart(*ch)) {
        Span span = getWhile(isIdentifierChar);
        std::string_view ident =
            std::string_view{input}.substr(span.begin, span.length());

        constexpr std::array<std::pair<std::string_view, Token>, 11> KEYWORDS{
            std::pair{"var", Token::Var},
            std::pair{"fun", Token::Fun},
            std::pair{"struct", Token::Struct},
            std::pair{"self", Token::Self},
            std::pair{"return", Token::Return},
            std::pair{"if", Token::If},
            std::pair{"else", Token::Else},
            std::pair{"while", Token::While},
            std::pair{"true", Token::True},
            std::pair{"false", Token::False},
            std::pair{"null", Token::Null},
        };
        for (const auto& [s, t] : KEYWORDS) {
            if (s == ident) {
                return Lexeme{.token = t, .span = span};
            }
        }

        return Lexeme{.token = Token::Id, .span = span};
    }

    throw std::runtime_error{"lexer encountered an unexpected character"};
}

std::optional<char> Lexer::peekChar() const {
    if (offset < input.size()) {
        return input[offset];
    }

    return std::nullopt;
}

std::optional<char> Lexer::nextChar() {
    auto res = peekChar();
    if (res.has_value()) {
        offset++;
    }
    return res;
}

void Lexer::rollback() {
    if (offset == 0)
        return;

    --offset;
}

std::string_view Lexer::peekChars(size_t n) const {
    return std::string_view{input}.substr(offset, n);
}
}  // namespace frontend::lex
