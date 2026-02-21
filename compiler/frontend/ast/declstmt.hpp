#pragma once

#include "compiler/frontend/ast/decl.hpp"
#include "compiler/frontend/ast/stmt.hpp"

namespace frontend::ast {
struct DeclStmt final : public Stmt {
    VarDecl decl;

    DeclStmt(VarDecl decl) : decl{std::move(decl)} {}
};
}  // namespace frontend::ast
