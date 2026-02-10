#pragma once

#include "decl.hpp"
#include "stmt.hpp"

namespace frontend::ast {
struct DeclStmt final : public Stmt {
    VarDecl decl;

    DeclStmt(VarDecl decl) : decl{std::move(decl)} {}
};
}  // namespace frontend::ast
