#pragma once

#include <memory>
#include <vector>

#include "frontend/ast/decl.hpp"
#include "frontend/ast/expr.hpp"
#include "frontend/ast/file.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/lex/lexeme.hpp"
#include "frontend/lex/span.hpp"
#include "frontend/lex/token.hpp"
#include "frontend/parse/error.hpp"

namespace frontend::parse {
class Parser {
    lex::Lexemes lexemes;

    template <typename P>
        requires std::predicate<P, lex::Token>
    bool eatIf(P pred) {
        auto l = lexemes.peek();
        bool res = pred(l.token);
        if (res) {
            lexemes.next();
        }
        return res;
    }

    lex::Span get(lex::Token tok);

    template <typename... Ts>
    lex::Lexeme get(lex::Token tok, Ts&&... expected) {
        auto l = lexemes.next();
        if (l.token == tok) {
            return l;
        }
        throw parse::error(l.token, std::forward<Ts...>(expected...));
    }

    template <typename P, typename... Ts>
        requires std::predicate<P, lex::Token>
    lex::Lexeme get(P pred, Ts&&... expected) {
        auto l = lexemes.next();
        if (pred(l.token)) {
            return l;
        }
        throw parse::error(l.token, std::forward<Ts...>(expected...));
    }

    template <typename P>
        requires std::invocable<P>
    std::vector<std::invoke_result_t<P>> parseManyUntil(P parser, lex::Token sep, lex::Token end) {
        std::vector<std::invoke_result_t<P>> res;

        for (;;) {
            lex::Lexeme l = lexemes.peek();
            if (l.token == end) {
                lexemes.next();
                break;
            }

            res.emplace_back(parser());
            l = lexemes.peek();
            if (l.token == sep) {
                lexemes.next();
                continue;
            }
            if (l.token == end) {
                lexemes.next();
                break;
            }

            throw parse::error(lexemes.getInput(), l.span, l.token, sep, end);
        }

        return res;
    }

    ast::VarDecl parseVarDecl();
    ast::FunctionDecl parseFunctionDecl();

    ast::CompoundStmt parseCompoundStmt();
    std::unique_ptr<ast::Stmt> parseStmt();

    std::unique_ptr<ast::Expr> parseTerm();
    std::unique_ptr<ast::Expr> parseExpr();

    ast::StructDecl parseStructDecl();

public:
    ast::File parse();

    Parser(lex::Lexemes lexemes) : lexemes{std::move(lexemes)} {}
};
}  // namespace frontend::parse
