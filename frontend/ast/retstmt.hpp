#pragma once

#include <memory>

#include "expr.hpp"
#include "stmt.hpp"

namespace frontend::ast {
struct RetStmt final : public Stmt {
    std::unique_ptr<Expr> value;

    RetStmt() : value{nullptr} {}
    RetStmt(std::unique_ptr<Expr> value) : value{std::move(value)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};
}  // namespace frontend::ast
