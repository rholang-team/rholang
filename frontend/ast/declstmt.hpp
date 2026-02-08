#pragma once

#include "decl.hpp"
#include "stmt.hpp"

namespace frontend::ast {
struct DeclStmt final : public Stmt {
    VarDecl decl;

    DeclStmt(VarDecl decl) : decl{std::move(decl)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};
}  // namespace frontend::ast
