#pragma once

#include <format>

namespace frontend::lex {
enum class Token {
    Eof,
    Id,
    Num,
    Assign,     // =
    Plus,       // +
    Minus,      // -
    Bang,       // !
    Asterisk,   // *
    Eq,         // ==
    Ne,         // !=
    Lt,         // <
    Gt,         // >
    Le,         // <=
    Ge,         // >=
    Dot,        // .
    Comma,      // ,
    Semicolon,  // ;
    Colon,      // :
    LParen,     // (
    RParen,     // )
    LBrace,     // {
    RBrace,     // }
    Var,        // `var`
    Fun,        // `fun`
    Struct,     // `struct`
    If,         // `if`
    Else,       // `else`
    While,      // `while`
    Return,     // `return`
};

}  // namespace frontend::lex

template <>
struct std::formatter<frontend::lex::Token> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(frontend::lex::Token token, std::format_context& ctx) const {
        switch (token) {
            case frontend::lex::Token::Eof:
                return std::format_to(ctx.out(), "EOF");
            case frontend::lex::Token::Id:
                return std::format_to(ctx.out(), "identifier");
            case frontend::lex::Token::Num:
                return std::format_to(ctx.out(), "number");
            case frontend::lex::Token::Assign:
                return std::format_to(ctx.out(), "`=`");
            case frontend::lex::Token::Plus:
                return std::format_to(ctx.out(), "`+`");
            case frontend::lex::Token::Minus:
                return std::format_to(ctx.out(), "`-`");
            case frontend::lex::Token::Bang:
                return std::format_to(ctx.out(), "`!`");
            case frontend::lex::Token::Asterisk:
                return std::format_to(ctx.out(), "`*`");
            case frontend::lex::Token::Eq:
                return std::format_to(ctx.out(), "`==`");
            case frontend::lex::Token::Ne:
                return std::format_to(ctx.out(), "`!=`");
            case frontend::lex::Token::Lt:
                return std::format_to(ctx.out(), "`<`");
            case frontend::lex::Token::Gt:
                return std::format_to(ctx.out(), "`>`");
            case frontend::lex::Token::Le:
                return std::format_to(ctx.out(), "`<=`");
            case frontend::lex::Token::Ge:
                return std::format_to(ctx.out(), "`>=`");
            case frontend::lex::Token::Dot:
                return std::format_to(ctx.out(), "`.`");
            case frontend::lex::Token::Comma:
                return std::format_to(ctx.out(), "`,`");
            case frontend::lex::Token::Colon:
                return std::format_to(ctx.out(), "`:`");
            case frontend::lex::Token::Semicolon:
                return std::format_to(ctx.out(), "`;`");
            case frontend::lex::Token::LParen:
                return std::format_to(ctx.out(), "`(`");
            case frontend::lex::Token::RParen:
                return std::format_to(ctx.out(), "`)`");
            case frontend::lex::Token::LBrace:
                return std::format_to(ctx.out(), "`{{`");
            case frontend::lex::Token::RBrace:
                return std::format_to(ctx.out(), "`}}`");
            case frontend::lex::Token::Fun:
                return std::format_to(ctx.out(), "`fun`");
            case frontend::lex::Token::Struct:
                return std::format_to(ctx.out(), "`struct`");
            case frontend::lex::Token::Var:
                return std::format_to(ctx.out(), "`var`");
            case frontend::lex::Token::If:
                return std::format_to(ctx.out(), "`if`");
            case frontend::lex::Token::Else:
                return std::format_to(ctx.out(), "`else`");
            case frontend::lex::Token::While:
                return std::format_to(ctx.out(), "`while`");
            case frontend::lex::Token::Return:
                return std::format_to(ctx.out(), "`return`");
        }
    }
};
