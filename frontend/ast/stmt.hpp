#pragma once

#include <memory>
#include <vector>

#include "frontend/ast/expr.hpp"

namespace frontend::ast {
struct Stmt  {
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
}  // namespace frontend::ast
