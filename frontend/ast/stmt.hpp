#pragma once

#include <memory>
#include <vector>

#include "frontend/ast/expr.hpp"

namespace frontend::ast {
struct Stmt {
    virtual ~Stmt() = default;
};

struct ExprStmt final : public Stmt {
    std::unique_ptr<Expr> expr;

    explicit ExprStmt(std::unique_ptr<Expr> expr) : expr{std::move(expr)} {}
};

struct RetStmt final : public Stmt {
    std::optional<std::unique_ptr<Expr>> value;

    RetStmt() : value{std::nullopt} {}
    explicit RetStmt(std::unique_ptr<Expr> value) : value{std::move(value)} {}
};

struct CompoundStmt final : public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;

    explicit CompoundStmt(std::vector<std::unique_ptr<Stmt>> stmts) : stmts{std::move(stmts)} {}
};

struct CondStmt final : public Stmt {
    std::unique_ptr<Expr> cond;
    CompoundStmt onTrue;
    std::optional<std::unique_ptr<Stmt>> onFalse;

    CondStmt(std::unique_ptr<Expr> cond,
             CompoundStmt onTrue,
             std::optional<std::unique_ptr<Stmt>> onFalse = std::nullopt)
        : cond{std::move(cond)}, onTrue{std::move(onTrue)}, onFalse{std::move(onFalse)} {}
};

struct WhileStmt final : public Stmt {
    std::unique_ptr<Expr> cond;
    CompoundStmt body;

    WhileStmt(std::unique_ptr<Expr> cond, CompoundStmt body)
        : cond{std::move(cond)}, body{std::move(body)} {}
};
}  // namespace frontend::ast
