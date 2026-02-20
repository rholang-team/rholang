#pragma once

#include <memory>
#include <vector>

#include "frontend/ast/expr.hpp"

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
    std::optional<std::shared_ptr<Expr>> value;

    RetStmt() : value{std::nullopt} {}
    explicit RetStmt(std::shared_ptr<Expr> value) : value{std::move(value)} {}
};

struct CompoundStmt final : public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;

    explicit CompoundStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : stmts{std::move(stmts)} {}
};

struct CondStmt final : public Stmt {
    std::shared_ptr<Expr> cond;
    CompoundStmt onTrue;
    std::optional<std::unique_ptr<Stmt>> onFalse;

    CondStmt(std::shared_ptr<Expr> cond,
             CompoundStmt onTrue,
             std::optional<std::unique_ptr<Stmt>> onFalse = std::nullopt)
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
