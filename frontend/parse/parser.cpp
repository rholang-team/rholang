#include "frontend/parse/parser.hpp"

#include <cassert>
#include <memory>
#include <print>
#include <stack>

#include "frontend/parse/error.hpp"

namespace frontend::parse {
namespace {
std::shared_ptr<Type> typeFromString(std::string_view s) {
    if (s == "void")
        return PrimitiveType::voidType;
    if (s == "bool")
        return PrimitiveType::boolType;
    if (s == "int")
        return PrimitiveType::intType;

    return std::make_shared<TypeRef>(std::string{s});
}

std::optional<ast::BinaryExpr::Op> binaryOpFromToken(lex::Token tok) {
    switch (tok) {
        case lex::Token::Assign:
            return ast::BinaryExpr::Op::Assign;
        case lex::Token::Plus:
            return ast::BinaryExpr::Op::Plus;
        case lex::Token::Minus:
            return ast::BinaryExpr::Op::Minus;
        case lex::Token::Asterisk:
            return ast::BinaryExpr::Op::Mul;
        case lex::Token::Eq:
            return ast::BinaryExpr::Op::Eq;
        default:
            return std::nullopt;
    }
}

std::pair<unsigned, bool> binaryOpInfo(ast::BinaryExpr::Op op) {
    bool leftAssociative = true;
    unsigned prec;
    switch (op) {
        case ast::BinaryExpr::Op::Assign:
            leftAssociative = false;
            prec = 1;
            break;
        case ast::BinaryExpr::Op::Eq:
            prec = 2;
            break;
        case ast::BinaryExpr::Op::Plus:
        case ast::BinaryExpr::Op::Minus:
            prec = 3;
            break;
        case ast::BinaryExpr::Op::Mul:
            prec = 4;
            break;
    }

    return {prec, leftAssociative};
}
}  // namespace

lex::Lexeme Parser::get(lex::Token tok) {
    auto l = lexemes.next();
    if (l.token == tok)
        return l;
    throw ParseError(lexemes.getInput(), l.span, l.token, tok);
}

std::vector<std::unique_ptr<ast::Decl>> Parser::parse() {
    std::vector<std::unique_ptr<ast::Decl>> res;

    for (;;) {
        auto decl = parseDecl();
        if (!decl)
            break;
        res.emplace_back(std::move(decl));
    }

    return res;
}

std::unique_ptr<ast::Decl> Parser::parseDecl() {
    auto first = lexemes.next();
    switch (first.token) {
        case lex::Token::Eof:
            return nullptr;
        case lex::Token::Fn: {
            lex::Lexeme name = get(lex::Token::Id);

            get(lex::Token::LParen);
            auto params = parseMany(
                [this]() {
                    auto name = get(lex::Token::Id);
                    auto type = get(lex::Token::Id);
                    return std::pair{lexemes.getLiteral(name.span),
                                     typeFromString(lexemes.getLiteral(type.span))};
                },
                lex::Token::Comma,
                lex::Token::RParen);

            auto tyLexeme = get(lex::Token::Id);
            auto ty = typeFromString(lexemes.getLiteral(tyLexeme.span));

            ast::CompoundStmt body = parseCompoundStmt();

            return std::make_unique<ast::FunctionDecl>(
                lexemes.getLiteral(name.span), std::move(params), ty, std::move(body));
        }
        case lex::Token::Var: {
            auto name = get(lex::Token::Id);
            auto tyLexeme = get(lex::Token::Id);
            auto ty = typeFromString(lexemes.getLiteral(tyLexeme.span));

            auto next = lexemes.next();
            if (next.token == lex::Token::Semicolon) {
                return std::make_unique<ast::VarDecl>(std::string{lexemes.getLiteral(name.span)},
                                                      ty,
                                                      std::unique_ptr<ast::Expr>{nullptr});
            } else if (next.token == lex::Token::Assign) {
                std::unique_ptr<ast::Expr> value = parseExpr();
                get(lex::Token::Semicolon);
                return std::make_unique<ast::VarDecl>(
                    std::string{lexemes.getLiteral(name.span)}, ty, std::move(value));
            }

            throw ParseError(lexemes.getInput(),
                             next.span,
                             next.token,
                             lex::Token::Semicolon,
                             lex::Token::Assign);
        }
        default:
            throw ParseError(lexemes.getInput(), first.span, first.token, "declaration");
    }

    std::unreachable();
}

ast::CompoundStmt Parser::parseCompoundStmt() {
    std::vector<std::unique_ptr<ast::Stmt>> stmts;

    get(lex::Token::LBrace);
    for (;;) {
        lex::Lexeme l = lexemes.peek();
        while (l.token == lex::Token::Semicolon) {
            lexemes.next();
            l = lexemes.peek();
        }
        if (l.token == lex::Token::RBrace) {
            lexemes.next();
            break;
        }
        // parseStmt should handle semicolons by itself
        stmts.emplace_back(parseStmt());
    }

    return ast::CompoundStmt{std::move(stmts)};
}

std::unique_ptr<ast::Stmt> Parser::parseStmt() {
    switch (lexemes.peek().token) {
        case lex::Token::LBrace:
            return std::make_unique<ast::CompoundStmt>(parseCompoundStmt());
        case lex::Token::If:
            throw std::runtime_error("not yet implemented");
        case lex::Token::Return: {
            lexemes.next();
            if (lexemes.peek().token == lex::Token::Semicolon) {
                lexemes.next();
                return std::make_unique<ast::RetStmt>();
            }
            auto res = parseExpr();
            get(lex::Token::Semicolon);
            return std::make_unique<ast::RetStmt>(std::move(res));
        }
        case lex::Token::Var: {
            lexemes.next();
            auto name = get(lex::Token::Id);
            auto tyLexeme = get(lex::Token::Id);
            auto ty = typeFromString(lexemes.getLiteral(tyLexeme.span));

            auto next = lexemes.next();
            if (next.token == lex::Token::Semicolon) {
                return std::make_unique<ast::DeclStmt>(
                    ast::VarDecl{std::string{lexemes.getLiteral(name.span)}, ty, nullptr});
            } else if (next.token == lex::Token::Assign) {
                std::unique_ptr<ast::Expr> value = parseExpr();
                get(lex::Token::Semicolon);
                return std::make_unique<ast::DeclStmt>(
                    ast::VarDecl(std::string{lexemes.getLiteral(name.span)}, ty, std::move(value)));
            }

            throw ParseError(lexemes.getInput(),
                             next.span,
                             next.token,
                             lex::Token::Semicolon,
                             lex::Token::Assign);
        }
        default: {
            auto res = parseExpr();
            get(lex::Token::Semicolon);
            return res;
        }
    }
}

std::unique_ptr<ast::Expr> Parser::parseExpr() {
    // https://www.nsl.com/k/parse/OperatorPrecedenceParsing.pdf

    std::stack<std::unique_ptr<ast::Expr>> terms;
    std::stack<ast::BinaryExpr::Op> ops;

    bool reduced = false;

    auto reduce = [&terms, &ops, &reduced]() {
        auto rhs = std::move(terms.top());
        terms.pop();
        auto lhs = std::move(terms.top());
        terms.pop();

        terms.emplace(std::make_unique<ast::BinaryExpr>(ops.top(), std::move(lhs), std::move(rhs)));
        ops.pop();

        reduced = true;
    };

    for (;;) {
        if (!reduced) {
            terms.emplace(parseTerm());
        }
        reduced = false;

        auto op = binaryOpFromToken(lexemes.peek().token);
        if (!op.has_value()) {
            while (!ops.empty()) {
                reduce();
            }

            assert(terms.size() == 1);
            return std::move(terms.top());
        }

        if (ops.empty()) {
            lexemes.next();
            ops.push(*op);
            continue;
        }

        auto [topOpPrec, topOpLeftAssoc] = binaryOpInfo(ops.top());
        auto [curOpPrec, curOpLeftAssoc] = binaryOpInfo(*op);

        if (topOpPrec < curOpPrec) {
            lexemes.next();
            ops.push(*op);
        } else if (topOpPrec > curOpPrec)
            reduce();
        else {
            assert(topOpPrec == curOpPrec);
            if (topOpLeftAssoc && curOpLeftAssoc)
                reduce();
            else if (!topOpLeftAssoc && !curOpLeftAssoc) {
                lexemes.next();
                ops.push(*op);
            } else {
                auto l = lexemes.next();
                throw ParseError::customMessage(
                    lexemes.getInput(),
                    l.span,
                    "cannot mix operators of different associativity and equal precedence");
            }
        }
    }
}

std::unique_ptr<ast::Expr> Parser::parseTerm() {
    auto l = lexemes.peek();

    // parse prefix ops
    bool minus = false;
    switch (l.token) {
        case lex::Token::Minus:
            lexemes.next();
            minus = true;
            break;
        default:
            break;
    }

    std::unique_ptr<ast::Expr> term;

    // parse the term itself
    l = lexemes.peek();
    switch (l.token) {
        case lex::Token::Id:
            lexemes.next();
            term = std::make_unique<ast::VarRefExpr>(lexemes.getLiteral(l.span));
            break;
        case lex::Token::Num: {
            lexemes.next();
            size_t value;
            auto lit = lexemes.getLiteral(l.span);
            auto res = std::from_chars(lit.data(), lit.data() + lit.size(), value);
            assert(res.ec == std::errc{});
            term = std::make_unique<ast::NumLitExpr>(value);
            break;
        }
        case lex::Token::LParen: {
            lexemes.next();
            auto subexpr = parseExpr();
            get(lex::Token::RParen);
            term = std::move(subexpr);
            break;
        }
        default:
            throw ParseError(lexemes.getInput(), l.span, l.token, "term");
    }

    // parse postfix ops
    for (;;) {
        auto l = lexemes.peek();
        if (l.token == lex::Token::Dot) {
            lexemes.next();
            auto id = get(lex::Token::Id);
            term =
                std::make_unique<ast::MemberRefExpr>(std::move(term), lexemes.getLiteral(id.span));
        } else if (l.token == lex::Token::LParen) {
            lexemes.next();
            auto args =
                parseMany([this]() { return parseExpr(); }, lex::Token::Comma, lex::Token::RParen);
            term = std::make_unique<ast::CallExpr>(std::move(term), std::move(args));
        } else {
            break;
        }
    }

    // add prefix ops
    if (minus) {
        term = std::make_unique<ast::UnaryExpr>(ast::UnaryExpr::Op::Minus, std::move(term));
    }
    return term;
}
}  // namespace frontend::parse
