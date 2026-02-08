#pragma once

#include <memory>
#include <vector>

#include "frontend/pretty.hpp"

namespace frontend::ast {
struct Stmt : public pretty::PrettyPrintable {
    virtual ~Stmt() = default;
};

struct CompoundStmt final : public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;

    explicit CompoundStmt(std::vector<std::unique_ptr<Stmt>> stmts) : stmts{std::move(stmts)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};
}  // namespace frontend::ast
