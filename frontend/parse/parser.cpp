#include "frontend/parse/parser.hpp"

#include <cassert>
#include <memory>
#include <stack>

#include "frontend/ast/declstmt.hpp"
#include "frontend/ast/file.hpp"
#include "frontend/lex/span.hpp"
#include "frontend/parse/error.hpp"

namespace frontend::parse {
namespace {
std::string mangleMethodName(std::string_view structName, std::string_view methodName) {
    return std::format("_R{}{}{}{}", structName.size(), structName, methodName.size(), methodName);
}

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

lex::Span Parser::get(lex::Token tok) {
    auto l = lexemes.next();
    if (l.token == tok)
        return l.span;
    throw parse::error(lexemes.getInput(), l.span, l.token, tok);
}

ast::File Parser::parse() {
    ast::File res;

    auto checkName = [this, &res](const lex::WithSpan<std::string>& name) {
        if (res.globals.contains(name.value) || res.functions.contains(name.value) ||
            res.structs.contains(name.value)) {
            throw Error(
                lexemes.getInput(), name.span, std::format("redeclaration of {}", name.value));
        }
    };

    for (;;) {
        auto l = lexemes.peek();
        if (l.token == lex::Token::Eof)
            break;
        else if (l.token == lex::Token::Fun) {
            auto decl = parseFunctionDecl();
            checkName(decl.name);
            res.functions.emplace(decl.name.value, std::move(decl));
        } else if (l.token == lex::Token::Var) {
            auto decl = parseVarDecl();
            checkName(decl.name);
            res.globals.emplace(decl.name.value, std::move(decl));
        } else if (l.token == lex::Token::Struct) {
            auto decl = parseStructDecl();
            checkName(decl.name);
            res.structs.emplace(decl.name.value, std::move(decl));
        } else {
            throw parse::error(lexemes.getInput(),
                               l.span,
                               l.token,
                               lex::Token::Fun,
                               lex::Token::Var,
                               lex::Token::Struct);
        }
    }

    return res;
}

ast::VarDecl Parser::parseVarDecl() {
    get(lex::Token::Var);

    auto nameSpan = get(lex::Token::Id);
    auto typeSpan = get(lex::Token::Id);
    auto type = typeFromString(lexemes.getLiteral(typeSpan));

    auto next = lexemes.next();
    if (next.token == lex::Token::Semicolon) {
        return ast::VarDecl{
            lex::WithSpan<std::string>{lexemes.getLiteral(nameSpan), nameSpan},
            lex::WithSpan{type, typeSpan},
            nullptr,
        };
    } else if (next.token == lex::Token::Assign) {
        std::unique_ptr<ast::Expr> value = parseExpr();
        get(lex::Token::Semicolon);
        return ast::VarDecl{
            lex::WithSpan<std::string>{lexemes.getLiteral(nameSpan), nameSpan},
            lex::WithSpan{type, typeSpan},
            std::move(value),
        };
    }

    throw parse::error(
        lexemes.getInput(), next.span, next.token, lex::Token::Semicolon, lex::Token::Assign);
}

ast::FunctionDecl Parser::parseFunctionDecl() {
    get(lex::Token::Fun);

    std::optional<std::pair<lex::WithSpan<std::string>, lex::WithSpan<std::shared_ptr<Type>>>>
        reciever;

    auto l = lexemes.peek();
    if (l.token == lex::Token::LParen) {
        lexemes.next();
        auto nameSpan = get(lex::Token::Id);
        auto typeSpan = get(lex::Token::Id);
        reciever = std::pair{lex::WithSpan<std::string>{lexemes.getLiteral(nameSpan), nameSpan},
                             lex::WithSpan{
                                 typeFromString(lexemes.getLiteral(typeSpan)),
                                 typeSpan,
                             }};
        get(lex::Token::RParen);
    }

    auto nameSpan = get(lex::Token::Id);

    get(lex::Token::LParen);
    auto params = parseManyUntil(
        [this]() {
            auto nameSpan = get(lex::Token::Id);
            auto typeSpan = get(lex::Token::Id);
            return std::pair{lex::WithSpan<std::string>{lexemes.getLiteral(nameSpan), nameSpan},
                             lex::WithSpan{
                                 typeFromString(lexemes.getLiteral(typeSpan)),
                                 typeSpan,
                             }};
        },
        lex::Token::Comma,
        lex::Token::RParen);

    std::string name = (reciever.has_value())
                           ? mangleMethodName(lexemes.getLiteral(reciever->second.span),
                                              lexemes.getLiteral(nameSpan))
                           : std::string{lexemes.getLiteral(nameSpan)};

    if (reciever.has_value()) {
        params.emplace(params.begin(), std::move(*reciever));
    }

    auto typeSpan = get(lex::Token::Id);
    auto type = typeFromString(lexemes.getLiteral(typeSpan));

    ast::CompoundStmt body = parseCompoundStmt();

    return ast::FunctionDecl{lex::WithSpan{std::move(name), nameSpan},
                             std::move(params),
                             lex::WithSpan{type, typeSpan},
                             std::move(body)};
}

ast::StructDecl Parser::parseStructDecl() {
    get(lex::Token::Struct);

    auto nameSpan = get(lex::Token::Id);

    get(lex::Token::LBrace);

    auto fields = parseManyUntil(
        [this]() {
            auto nameSpan = get(lex::Token::Id);
            auto typeSpan = get(lex::Token::Id);
            return ast::StructDecl::Field{lexemes.getLiteral(nameSpan),
                                          lex::WithSpan{
                                              typeFromString(lexemes.getLiteral(typeSpan)),
                                              typeSpan,
                                          }};
        },
        lex::Token::Comma,
        lex::Token::RBrace);

    return ast::StructDecl{
        lex::WithSpan<std::string>{lexemes.getLiteral(nameSpan), nameSpan},
        std::move(fields),
    };
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
            throw std::runtime_error("TODO");
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
        case lex::Token::Var:
            return std::make_unique<ast::DeclStmt>(parseVarDecl());
        default: {
            auto res = parseExpr();
            get(lex::Token::Semicolon);
            return std::make_unique<ast::ExprStmt>(std::move(res));
        }
    }
}

std::unique_ptr<ast::Expr> Parser::parseExpr() {
    // https://www.nsl.com/k/parse/OperatorPrecedenceParsing.pdf

    std::stack<std::unique_ptr<ast::Expr>> terms;
    std::stack<lex::WithSpan<ast::BinaryExpr::Op>> ops;

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

        auto opLexeme = lexemes.peek();
        auto op = binaryOpFromToken(opLexeme.token);
        if (!op.has_value()) {
            while (!ops.empty()) {
                reduce();
            }

            assert(terms.size() == 1);
            return std::move(terms.top());
        }

        if (ops.empty()) {
            lexemes.next();
            ops.emplace(*op, opLexeme.span);
            continue;
        }

        auto [topOpPrec, topOpLeftAssoc] = binaryOpInfo(ops.top().value);
        auto [curOpPrec, curOpLeftAssoc] = binaryOpInfo(*op);

        if (topOpPrec < curOpPrec) {
            lexemes.next();
            ops.emplace(*op, opLexeme.span);
        } else if (topOpPrec > curOpPrec)
            reduce();
        else {
            assert(topOpPrec == curOpPrec);
            if (topOpLeftAssoc && curOpLeftAssoc)
                reduce();
            else if (!topOpLeftAssoc && !curOpLeftAssoc) {
                lexemes.next();
                ops.emplace(*op, opLexeme.span);
            } else {
                auto l = lexemes.next();
                throw Error(lexemes.getInput(),
                            l.span,
                            "cannot mix operators of different associativity and equal precedence");
            }
        }
    }
}

std::unique_ptr<ast::Expr> Parser::parseTerm() {
    auto l = lexemes.peek();

    // parse prefix ops
    std::optional<lex::Span> unaryMinusSpan = std::nullopt;
    switch (l.token) {
        case lex::Token::Minus:
            unaryMinusSpan = lexemes.next().span;
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
            term = std::make_unique<ast::VarRefExpr>(
                lex::WithSpan<std::string>{lexemes.getLiteral(l.span), l.span});
            break;
        case lex::Token::Num: {
            lexemes.next();
            size_t value;
            auto lit = lexemes.getLiteral(l.span);
            auto res = std::from_chars(lit.data(), lit.data() + lit.size(), value);
            assert(res.ec == std::errc{});
            term = std::make_unique<ast::NumLitExpr>(lex::WithSpan{value, l.span});
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
            throw parse::error(lexemes.getInput(), l.span, l.token, "term");
    }

    // parse postfix ops
    for (;;) {
        auto l = lexemes.peek();
        if (l.token == lex::Token::Dot) {
            lexemes.next();
            auto idSpan = get(lex::Token::Id);
            term = std::make_unique<ast::MemberRefExpr>(
                std::move(term), lex::WithSpan<std::string>{lexemes.getLiteral(idSpan), idSpan});
        } else if (l.token == lex::Token::LParen) {
            lexemes.next();
            auto args = parseManyUntil(
                [this]() { return parseExpr(); }, lex::Token::Comma, lex::Token::RParen);
            term = std::make_unique<ast::CallExpr>(std::move(term), std::move(args));
        } else {
            break;
        }
    }

    // add prefix ops
    if (unaryMinusSpan.has_value()) {
        term = std::make_unique<ast::UnaryExpr>(
            lex::WithSpan{ast::UnaryExpr::Op::Minus, *unaryMinusSpan}, std::move(term));
    }
    return term;
}
}  // namespace frontend::parse
