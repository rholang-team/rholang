#pragma once

#include <forward_list>
#include <string>
#include <unordered_map>

#include "frontend/ast/decl.hpp"
#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/ast/visitor.hpp"
#include "frontend/translationunit.hpp"

namespace frontend {
class Sema : private ast::DeclVisitor<void>, ast::StmtVisitor<void>, ast::ExprVisitor<void> {
    using DeclVisitor<void>::visit;
    using StmtVisitor<void>::visit;
    using ExprVisitor<void>::visit;

    std::string_view input;
    std::forward_list<std::unordered_map<std::string, std::shared_ptr<Type>>> scopes;
    ast::FunctionDecl* curFunction = nullptr;

    void pushScope();
    void popScope();

    void addToScope(std::string name, std::shared_ptr<Type> type);

    std::optional<std::shared_ptr<Type>> lookup(const std::string& name) const;

    void visit(ast::FunctionDecl& decl) override;
    void visit(ast::VarDecl& decl) override;

    void visit(ast::CompoundStmt& stmt) override;
    void visit(ast::DeclStmt& stmt) override;
    void visit(ast::RetStmt& stmt) override;
    void visit(ast::ExprStmt& stmt) override;

    void visit(ast::UnaryExpr& expr) override;
    void visit(ast::NumLitExpr& expr) override;
    void visit(ast::BinaryExpr& expr) override;
    void visit(ast::VarRefExpr& expr) override;
    void visit(ast::MemberRefExpr& expr) override;
    void visit(ast::CallExpr& expr) override;

public:
    Sema(std::string_view input) : input{input} {}

    void run(TranslationUnit& tu);
};
}  // namespace frontend
