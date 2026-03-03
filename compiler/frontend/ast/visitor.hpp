#pragma once

#include "compiler/frontend/ast/decl.hpp"
#include "compiler/frontend/ast/declstmt.hpp"
#include "compiler/frontend/ast/expr.hpp"
#include "compiler/frontend/ast/stmt.hpp"

namespace frontend::ast {
struct DeclVisitor {
    virtual ~DeclVisitor() = default;

    virtual void visit([[maybe_unused]] FunctionDecl& decl) {}
    virtual void visit([[maybe_unused]] VarDecl& decl) {}
    virtual void visit([[maybe_unused]] StructDecl& decl) {}
};

template <typename RetTy>
struct StmtVisitor {
    virtual ~StmtVisitor() = default;

    virtual RetTy visit(Stmt* stmt) {
        if (auto* compoundStmt = dynamic_cast<CompoundStmt*>(stmt))
            return visit(*compoundStmt);
        else if (auto* condStmt = dynamic_cast<CondStmt*>(stmt))
            return visit(*condStmt);
        else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt))
            return visit(*whileStmt);
        else if (auto* declStmt = dynamic_cast<DeclStmt*>(stmt))
            return visit(*declStmt);
        else if (auto* retStmt = dynamic_cast<RetStmt*>(stmt))
            return visit(*retStmt);
        else if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt))
            return visit(*exprStmt);
        else if (auto* assignmentStmt = dynamic_cast<AssignmentStmt*>(stmt))
            return visit(*assignmentStmt);

        std::unreachable();
    }

    virtual RetTy visit(CompoundStmt&) {}
    virtual RetTy visit(CondStmt&) {}
    virtual RetTy visit(WhileStmt&) {}
    virtual RetTy visit(DeclStmt&) {}
    virtual RetTy visit(RetStmt&) {}
    virtual RetTy visit(ExprStmt&) {}
    virtual RetTy visit(AssignmentStmt&) {}
};

template <typename RetTy>
struct ExprVisitor {
    virtual ~ExprVisitor() = default;

    virtual RetTy visit(Expr* expr) {
        if (auto* unaryExpr = dynamic_cast<UnaryExpr*>(expr)) {
            return visit(*unaryExpr);
        } else if (auto* numLitExpr = dynamic_cast<NumLitExpr*>(expr)) {
            return visit(*numLitExpr);
        } else if (auto* boolLitExpr = dynamic_cast<BoolLitExpr*>(expr)) {
            return visit(*boolLitExpr);
        } else if (auto* nullExpr = dynamic_cast<NullExpr*>(expr)) {
            return visit(*nullExpr);
        } else if (auto* binaryExpr = dynamic_cast<BinaryExpr*>(expr)) {
            return visit(*binaryExpr);
        } else if (auto* varRefExpr = dynamic_cast<VarRefExpr*>(expr)) {
            return visit(*varRefExpr);
        } else if (auto* memberRefExpr = dynamic_cast<MemberRefExpr*>(expr)) {
            return visit(*memberRefExpr);
        } else if (auto* callExpr = dynamic_cast<CallExpr*>(expr)) {
            return visit(*callExpr);
        } else if (auto* initExpr = dynamic_cast<StructInitExpr*>(expr)) {
            return visit(*initExpr);
        }

        std::unreachable();
    }

    virtual RetTy visit(UnaryExpr&) {}
    virtual RetTy visit(NumLitExpr&) {}
    virtual RetTy visit(BoolLitExpr&) {}
    virtual RetTy visit(NullExpr&) {}
    virtual RetTy visit(BinaryExpr&) {}
    virtual RetTy visit(VarRefExpr&) {}
    virtual RetTy visit(MemberRefExpr&) {}
    virtual RetTy visit(CallExpr&) {}
    virtual RetTy visit(StructInitExpr&) {}
};
}  // namespace frontend::ast
