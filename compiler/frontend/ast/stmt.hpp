#pragma once

#include <memory>
#include <vector>

#include "compiler/frontend/ast/expr.hpp"

namespace frontend::ast {
struct Stmt {
    virtual ~Stmt() = default;
};

struct AssignmentStmt final : public Stmt {
    lex::Span opSpan;
    std::shared_ptr<Expr> lhs;
    std::shared_ptr<Expr> rhs;

    AssignmentStmt(lex::Span opSpan,
                   std::shared_ptr<Expr> lhs,
                   std::shared_ptr<Expr> rhs)
        : opSpan{opSpan}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {};
};

struct ExprStmt final : public Stmt {
    std::shared_ptr<Expr> expr;

    explicit ExprStmt(std::shared_ptr<Expr> expr) : expr{std::move(expr)} {}
};

struct RetStmt final : public Stmt {
    lex::Span span;
    std::optional<std::shared_ptr<Expr>> value;

    explicit RetStmt(lex::Span span) : span{span}, value{std::nullopt} {}
    RetStmt(lex::Span span, std::shared_ptr<Expr> value)
        : span{span}, value{std::move(value)} {}
};

struct CompoundStmt final : public Stmt {
    std::vector<std::shared_ptr<Stmt>> stmts;

    explicit CompoundStmt(std::vector<std::shared_ptr<Stmt>> stmts)
        : stmts{std::move(stmts)} {}
};

struct CondStmt final : public Stmt {
    std::shared_ptr<Expr> cond;
    CompoundStmt onTrue;
    std::optional<std::shared_ptr<Stmt>> onFalse;

    CondStmt(std::shared_ptr<Expr> cond,
             CompoundStmt onTrue,
             std::optional<std::shared_ptr<Stmt>> onFalse = std::nullopt)
        : cond{std::move(cond)},
          onTrue{std::move(onTrue)},
          onFalse{std::move(onFalse)} {}
};

struct WhileStmt final : public Stmt {
    std::shared_ptr<Expr> cond;
    CompoundStmt body;

    WhileStmt(std::shared_ptr<Expr> cond, CompoundStmt body)
        : cond{std::move(cond)}, body{std::move(body)} {}
};
}  // namespace frontend::ast
