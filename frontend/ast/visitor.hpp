#pragma once

#include "frontend/ast/decl.hpp"
#include "frontend/ast/declstmt.hpp"
#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"

namespace frontend::ast {
template <typename RetTy>
struct DeclVisitor {
    virtual ~DeclVisitor() = default;

    virtual RetTy visit(Decl* decl) {
        if (auto* fndecl = dynamic_cast<FunctionDecl*>(decl)) {
            return visit(*fndecl);
        } else if (auto* vardecl = dynamic_cast<VarDecl*>(decl)) {
            return visit(*vardecl);
        }

        std::unreachable();
    }

    virtual RetTy visit([[maybe_unused]] FunctionDecl& decl) {}
    virtual RetTy visit([[maybe_unused]] VarDecl& decl) {}
};

template <typename RetTy>
struct StmtVisitor {
    virtual ~StmtVisitor() = default;

    virtual RetTy visit(Stmt* stmt) {
        if (auto* compoundStmt = dynamic_cast<CompoundStmt*>(stmt)) {
            return visit(*compoundStmt);
        } else if (auto* declStmt = dynamic_cast<DeclStmt*>(stmt)) {
            return visit(*declStmt);
        } else if (auto* retStmt = dynamic_cast<RetStmt*>(stmt)) {
            return visit(*retStmt);
        } else if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
            return visit(*exprStmt);
        }

        std::unreachable();
    }

    virtual RetTy visit([[maybe_unused]] CompoundStmt& stmt) {}
    virtual RetTy visit([[maybe_unused]] DeclStmt& stmt) {}
    virtual RetTy visit([[maybe_unused]] RetStmt& stmt) {}
    virtual RetTy visit([[maybe_unused]] ExprStmt& stmt) {}
};

template <typename RetTy>
struct ExprVisitor {
    virtual ~ExprVisitor() = default;

    virtual RetTy visit(Expr* expr) {
        if (auto* unaryExpr = dynamic_cast<UnaryExpr*>(expr)) {
            return visit(*unaryExpr);
        } else if (auto* numLitExpr = dynamic_cast<NumLitExpr*>(expr)) {
            return visit(*numLitExpr);
        } else if (auto* binaryExpr = dynamic_cast<BinaryExpr*>(expr)) {
            return visit(*binaryExpr);
        } else if (auto* varRefExpr = dynamic_cast<VarRefExpr*>(expr)) {
            return visit(*varRefExpr);
        } else if (auto* memberRefExpr = dynamic_cast<MemberRefExpr*>(expr)) {
            return visit(*memberRefExpr);
        } else if (auto* callExpr = dynamic_cast<CallExpr*>(expr)) {
            return visit(*callExpr);
        }

        std::unreachable();
    }

    virtual RetTy visit([[maybe_unused]] UnaryExpr& expr) {}
    virtual RetTy visit([[maybe_unused]] NumLitExpr& expr) {}
    virtual RetTy visit([[maybe_unused]] BinaryExpr& expr) {}
    virtual RetTy visit([[maybe_unused]] VarRefExpr& expr) {}
    virtual RetTy visit([[maybe_unused]] MemberRefExpr& expr) {}
    virtual RetTy visit([[maybe_unused]] CallExpr& expr) {}
};
}  // namespace frontend::ast
