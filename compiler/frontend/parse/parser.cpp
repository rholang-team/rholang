#include "compiler/frontend/parse/parser.hpp"

#include <cassert>
#include <memory>
#include <stack>
#include <unordered_set>

#include "compiler/frontend/ast/declstmt.hpp"
#include "compiler/frontend/ast/file.hpp"
#include "compiler/frontend/lex/span.hpp"
#include "compiler/frontend/mangling.hpp"
#include "compiler/frontend/parse/error.hpp"

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

std::optional<ast::UnaryExpr::Op> unaryOpFromToken(lex::Token tok) {
    switch (tok) {
        case lex::Token::Bang:
            return ast::UnaryExpr::Op::Not;
        case lex::Token::Minus:
            return ast::UnaryExpr::Op::Minus;
        default:
            return std::nullopt;
    }
}

std::optional<ast::BinaryExpr::Op> binaryOpFromToken(lex::Token tok) {
    switch (tok) {
        case lex::Token::Plus:
            return ast::BinaryExpr::Op::Plus;
        case lex::Token::Minus:
            return ast::BinaryExpr::Op::Minus;
        case lex::Token::Asterisk:
            return ast::BinaryExpr::Op::Mul;
        case lex::Token::Eq:
            return ast::BinaryExpr::Op::Eq;
        case lex::Token::Ne:
            return ast::BinaryExpr::Op::Ne;
        case lex::Token::Lt:
            return ast::BinaryExpr::Op::Lt;
        case lex::Token::Gt:
            return ast::BinaryExpr::Op::Gt;
        case lex::Token::Le:
            return ast::BinaryExpr::Op::Le;
        case lex::Token::Ge:
            return ast::BinaryExpr::Op::Ge;
        default:
            return std::nullopt;
    }
}

std::pair<unsigned, bool> binaryOpInfo(ast::BinaryExpr::Op op) {
    bool leftAssociative = true;
    unsigned prec;
    switch (op) {
        case ast::BinaryExpr::Op::Or:
            prec = 1;
            break;
        case ast::BinaryExpr::Op::And:
            prec = 2;
            break;
        case ast::BinaryExpr::Op::Eq:
        case ast::BinaryExpr::Op::Ne:
            prec = 3;
            break;
        case ast::BinaryExpr::Op::Lt:
        case ast::BinaryExpr::Op::Gt:
        case ast::BinaryExpr::Op::Le:
        case ast::BinaryExpr::Op::Ge:
            prec = 4;
            break;
        case ast::BinaryExpr::Op::Plus:
        case ast::BinaryExpr::Op::Minus:
            prec = 5;
            break;
        case ast::BinaryExpr::Op::Mul:
            prec = 6;
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
    res.input = lexemes.getInput();

    auto checkName = [this, &res](const lex::WithSpan<std::string>& name) {
        if (res.globals.contains(name.value) ||
            res.functions.contains(name.value) ||
            res.structs.contains(name.value)) {
            throw Error(lexemes.getInput(),
                        name.span,
                        std::format("redeclaration of `{}`", name.value));
        }
    };

    for (;;) {
        auto l = lexemes.peek();
        if (l.token == lex::Token::Eof)
            break;
        else if (l.token == lex::Token::Fun) {
            auto decl = parseFunctionDecl();
            checkName(decl->name);
            res.functions.emplace(decl->name.value, std::move(decl));
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

    get(lex::Token::Assign);
    std::shared_ptr<ast::Expr> value = parseExpr();

    get(lex::Token::Semicolon);

    return ast::VarDecl{
        lex::WithSpan<std::string>{lexemes.getLiteral(nameSpan), nameSpan},
        lex::WithSpan{type, typeSpan},
        value,
    };
}

std::shared_ptr<ast::FunctionDecl> Parser::parseFunctionDecl() {
    get(lex::Token::Fun);

    auto nameSpan = get(lex::Token::Id);

    get(lex::Token::LParen);

    auto l = lexemes.peek();

    std::vector<std::string> paramNames;
    std::vector<lex::WithSpan<std::shared_ptr<Type>>> paramTypes;

    if (l.token == lex::Token::Self) {
        lexemes.next();
        paramNames.emplace_back("self");
        paramTypes.emplace_back(
            lex::WithSpan{std::shared_ptr<Type>{nullptr}, l.span});

        l = lexemes.peek();
        if (l.token == lex::Token::Comma) {
            lexemes.next();
        }
    }

    std::unordered_set<std::string> seenParamNames;

    parseManyUntil(
        [this, &paramNames, &paramTypes, &seenParamNames]() {
            auto nameSpan = get(lex::Token::Id);
            auto typeSpan = get(lex::Token::Id);

            std::string name{lexemes.getLiteral(nameSpan)};

            if (seenParamNames.contains(name)) {
                throw Error(
                    lexemes.getInput(),
                    nameSpan,
                    std::format("redefinition of parameter `{}`", name));
            }

            seenParamNames.insert(name);

            paramNames.emplace_back(std::move(name));
            paramTypes.emplace_back(
                typeFromString(lexemes.getLiteral(typeSpan)),
                typeSpan);
        },
        lex::Token::Comma,
        lex::Token::RParen);

    std::string name{lexemes.getLiteral(nameSpan)};

    auto typeSpan = get(lex::Token::Id);
    auto type = typeFromString(lexemes.getLiteral(typeSpan));

    ast::CompoundStmt body = parseCompoundStmt();

    return std::make_shared<ast::FunctionDecl>(
        lex::WithSpan{std::move(name), nameSpan},
        std::move(paramNames),
        std::move(paramTypes),
        lex::WithSpan{type, typeSpan},
        std::move(body));
}

ast::StructDecl Parser::parseStructDecl() {
    get(lex::Token::Struct);

    auto nameSpan = get(lex::Token::Id);
    std::string structName{lexemes.getLiteral(nameSpan)};

    get(lex::Token::LBrace);

    std::unordered_set<std::string> seenMemberNames;

    auto checkName =
        [this, &seenMemberNames](const lex::WithSpan<std::string>& name) {
            if (seenMemberNames.contains(name.value)) {
                throw Error(
                    lexemes.getInput(),
                    name.span,
                    std::format("redeclaration of member `{}`", name.value));
            }
        };

    std::vector<ast::StructDecl::Field> fields;
    std::unordered_map<std::string, std::shared_ptr<ast::FunctionDecl>> methods;

    for (;;) {
        auto l = lexemes.peek();
        if (l.token == lex::Token::RBrace)
            break;
        else if (l.token == lex::Token::Fun) {
            auto decl = parseFunctionDecl();
            if (!decl->isInstanceMethod()) {
                throw Error(lexemes.getInput(),
                            decl->name.span,
                            "static methods are not supported");
            }

            checkName(decl->name);
            seenMemberNames.insert(decl->name.value);

            std::string originalName = decl->name.value;
            decl->name.value = mangleMethodName(structName, originalName);
            methods.emplace(originalName, std::move(decl));
        } else if (l.token == lex::Token::Var) {
            lexemes.next();
            auto nameSpan = get(lex::Token::Id);
            auto typeSpan = get(lex::Token::Id);
            get(lex::Token::Semicolon);

            lex::WithSpan<std::string> name{
                std::string{lexemes.getLiteral(nameSpan)},
                nameSpan,
            };

            lex::WithSpan<std::shared_ptr<Type>> type{
                typeFromString(lexemes.getLiteral(typeSpan)),
                typeSpan,
            };

            checkName(name);
            seenMemberNames.insert(name.value);
            fields.emplace_back(std::move(name), type);
        } else {
            throw parse::error(lexemes.getInput(),
                               l.span,
                               l.token,
                               lex::Token::RBrace,
                               "field",
                               "method");
        }
    }

    get(lex::Token::RBrace);

    return ast::StructDecl{
        lex::WithSpan<std::string>{structName, nameSpan},
        std::move(fields),
        std::move(methods),
    };
}

ast::CompoundStmt Parser::parseCompoundStmt() {
    std::vector<std::shared_ptr<ast::Stmt>> stmts;

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

ast::CondStmt Parser::parseCondStmt() {
    get(lex::Token::If);

    get(lex::Token::LParen);
    auto cond = parseExpr();
    get(lex::Token::RParen);

    auto onTrue = parseCompoundStmt();

    std::optional<std::shared_ptr<ast::Stmt>> onFalse;

    if (lexemes.peek().token == lex::Token::Else) {
        lexemes.next();

        auto l = lexemes.peek();
        if (l.token == lex::Token::If) {
            onFalse = std::make_shared<ast::CondStmt>(parseCondStmt());
        } else if (l.token == lex::Token::LBrace) {
            onFalse = std::make_shared<ast::CompoundStmt>(parseCompoundStmt());
        } else {
            throw parse::error(lexemes.getInput(),
                               l.span,
                               l.token,
                               lex::Token::If,
                               lex::Token::LBrace);
        }
    }

    return ast::CondStmt{std::move(cond),
                         std::move(onTrue),
                         std::move(onFalse)};
}

ast::WhileStmt Parser::parseWhileStmt() {
    get(lex::Token::While);

    get(lex::Token::LParen);
    auto cond = parseExpr();
    get(lex::Token::RParen);

    auto body = parseCompoundStmt();

    return ast::WhileStmt{cond, body};
}

std::shared_ptr<ast::Stmt> Parser::parseExprOrAssignment() {
    auto lhs = parseExpr();
    auto l = lexemes.peek();

    if (l.token == lex::Token::Semicolon) {
        lexemes.next();
        return std::make_shared<ast::ExprStmt>(std::move(lhs));
    }

    std::optional<ast::BinaryExpr::Op> assigmentOp;

    switch (l.token) {
        case lex::Token::Assign:
            break;
        case lex::Token::PlusAssign:
            assigmentOp = ast::BinaryExpr::Op::Plus;
            break;
        case lex::Token::MinusAssign:
            assigmentOp = ast::BinaryExpr::Op::Minus;
            break;
        case lex::Token::MulAssign:
            assigmentOp = ast::BinaryExpr::Op::Mul;
            break;
        default:
            throw parse::error(lexemes.getInput(),
                               l.span,
                               l.token,
                               lex::Token::Semicolon,
                               "assignment");
    }

    lexemes.next();
    auto rhs = parseExpr();
    get(lex::Token::Semicolon);

    if (assigmentOp.has_value()) {
        rhs = std::make_shared<ast::BinaryExpr>(
            lex::WithSpan{*assigmentOp, l.span},
            lhs,
            rhs);
    }

    return std::make_shared<ast::AssignmentStmt>(l.span,
                                                 std::move(lhs),
                                                 std::move(rhs));
}

std::shared_ptr<ast::Stmt> Parser::parseStmt() {
    switch (lexemes.peek().token) {
        case lex::Token::LBrace:
            return std::make_shared<ast::CompoundStmt>(parseCompoundStmt());
        case lex::Token::If:
            return std::make_shared<ast::CondStmt>(parseCondStmt());
        case lex::Token::While:
            return std::make_shared<ast::WhileStmt>(parseWhileStmt());
        case lex::Token::Return: {
            auto retspan = lexemes.next().span;
            if (lexemes.peek().token == lex::Token::Semicolon) {
                lexemes.next();
                return std::make_shared<ast::RetStmt>(retspan);
            }
            auto res = parseExpr();
            get(lex::Token::Semicolon);
            return std::make_shared<ast::RetStmt>(retspan, std::move(res));
        }
        case lex::Token::Var:
            return std::make_shared<ast::DeclStmt>(parseVarDecl());
        default:
            return parseExprOrAssignment();
    }
}

std::shared_ptr<ast::Expr> Parser::parseExpr() {
    // https://www.nsl.com/k/parse/OperatorPrecedenceParsing.pdf

    std::stack<std::shared_ptr<ast::Expr>> terms;
    std::stack<lex::WithSpan<ast::BinaryExpr::Op>> ops;

    bool reduced = false;

    auto reduce = [&terms, &ops, &reduced]() {
        auto rhs = std::move(terms.top());
        terms.pop();
        auto lhs = std::move(terms.top());
        terms.pop();

        terms.emplace(std::make_shared<ast::BinaryExpr>(ops.top(),
                                                        std::move(lhs),
                                                        std::move(rhs)));
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
                            "cannot mix operators of different associativity "
                            "and equal precedence");
            }
        }
    }
}

std::shared_ptr<ast::Expr> Parser::parseTerm() {
    // prefix ops
    std::vector<lex::WithSpan<ast::UnaryExpr::Op>> ops;
    while (auto op = unaryOpFromToken(lexemes.peek().token)) {
        auto span = lexemes.next().span;
        if (std::ranges::find_if(ops, [op](const auto& seenOp) {
                return seenOp.value == *op;
            }) != ops.end()) {
            throw Error(lexemes.getInput(),
                        span,
                        "unary operator cannot be applied twice");
        }

        ops.emplace_back(*op, span);
    }

    // the term itself
    std::shared_ptr<ast::Expr> term;

    auto l = lexemes.peek();
    auto parseStructInitFields = [this, &l]() {
        std::unordered_map<std::string, std::shared_ptr<ast::Expr>> fields;

        parseManyUntil(
            [this, &fields]() {
                get(lex::Token::Dot);
                auto nameSpan = get(lex::Token::Id);
                get(lex::Token::Assign);
                auto value = parseExpr();

                std::string name{lexemes.getLiteral(nameSpan)};
                if (fields.contains(name)) {
                    throw Error(
                        lexemes.getInput(),
                        nameSpan,
                        std::format("double initialization of field `{}`",
                                    name));
                }

                fields.emplace(std::move(name), std::move(value));
            },
            lex::Token::Comma,
            lex::Token::RBrace);

        return ast::StructInitExpr{
            lex::WithSpan<std::shared_ptr<Type>>{
                std::make_shared<TypeRef>(lexemes.getLiteral(l.span)),
                l.span},
            std::move(fields)};
    };

    l = lexemes.peek();
    switch (l.token) {
        case lex::Token::Id:
            lexemes.next();
            if (lexemes.peek().token == lex::Token::LBrace) {
                lexemes.next();
                term = std::make_shared<ast::StructInitExpr>(
                    parseStructInitFields());
            } else {
                term = std::make_shared<ast::VarRefExpr>(
                    lex::WithSpan<std::string>{lexemes.getLiteral(l.span),
                                               l.span});
            }
            break;
        case lex::Token::Self: {
            lexemes.next();
            term = std::make_shared<ast::VarRefExpr>(
                lex::WithSpan<std::string>{"self", l.span});
            break;
        }
        case lex::Token::Num: {
            lexemes.next();
            unsigned value;
            auto lit = lexemes.getLiteral(l.span);
            auto res =
                std::from_chars(lit.data(), lit.data() + lit.size(), value);
            assert(res.ec == std::errc{});
            term =
                std::make_shared<ast::NumLitExpr>(lex::WithSpan{value, l.span});
            break;
        }
        case lex::Token::True:
            lexemes.next();
            term =
                std::make_shared<ast::BoolLitExpr>(lex::WithSpan{true, l.span});
            break;
        case lex::Token::False:
            lexemes.next();
            term = std::make_shared<ast::BoolLitExpr>(
                lex::WithSpan{false, l.span});
            break;
        case lex::Token::Null:
            lexemes.next();
            term = std::make_shared<ast::NullExpr>(l.span);
            break;
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

    // postfix ops
    for (;;) {
        auto l = lexemes.peek();
        if (l.token == lex::Token::Dot) {
            lexemes.next();
            auto idSpan = get(lex::Token::Id);
            term = std::make_shared<ast::MemberRefExpr>(
                std::move(term),
                lex::WithSpan<std::string>{lexemes.getLiteral(idSpan), idSpan});
        } else if (l.token == lex::Token::LParen) {
            lexemes.next();
            auto args = parseManyUntil([this]() { return parseExpr(); },
                                       lex::Token::Comma,
                                       lex::Token::RParen);
            term = std::make_shared<ast::CallExpr>(std::move(term),
                                                   std::move(args));
        } else {
            break;
        }
    }

    // add prefix ops
    for (const auto& op : std::ranges::reverse_view{ops}) {
        term = std::make_shared<ast::UnaryExpr>(op, std::move(term));
    }

    return term;
}
}  // namespace frontend::parse
