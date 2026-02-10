#pragma once

#include <ostream>

#include "frontend/ast/visitor.hpp"

namespace frontend::ast {
class PrettyPrinter : public DeclVisitor<void>, StmtVisitor<void>, ExprVisitor<void> {
    unsigned depth = 0;
    std::ostream& os;

    void showTyPtr(frontend::Type* ty);
    void pad();

public:
    using DeclVisitor<void>::visit;
    using StmtVisitor<void>::visit;
    using ExprVisitor<void>::visit;

    PrettyPrinter(std::ostream& os) : os{os} {}

    void visit(Decl* decl) override;
    void visit(Stmt* stmt) override;
    void visit(Expr* expr) override;

    void visit(FunctionDecl& decl) override;
    void visit(VarDecl& decl) override;

    void visit(CompoundStmt& stmt) override;
    void visit(DeclStmt& stmt) override;
    void visit(RetStmt& stmt) override;
    void visit(ExprStmt& stmt) override;

    void visit(UnaryExpr& expr) override;
    void visit(NumLitExpr& expr) override;
    void visit(BinaryExpr& expr) override;
    void visit(VarRefExpr& expr) override;
    void visit(MemberRefExpr& expr) override;
    void visit(CallExpr& expr) override;
};
}  // namespace frontend::ast
