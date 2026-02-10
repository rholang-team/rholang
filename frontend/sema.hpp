#pragma once

#include <forward_list>
#include <string>
#include <unordered_map>

#include "frontend/ast/decl.hpp"
#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/translationunit.hpp"

namespace frontend {
class Sema {
    std::string_view input;
    std::forward_list<std::unordered_map<std::string, std::shared_ptr<Type>>> scopes;

    void pushScope();
    void popScope();

    void addToScope(std::string name, std::shared_ptr<Type> type);

    std::optional<std::shared_ptr<Type>> lookup(const std::string& name) const;

    void run(ast::Expr* expr);

    void run(ast::CompoundStmt& stmt);
    void run(ast::Stmt* stmt);

    void run(ast::Decl* decl);

public:
    Sema(std::string_view input) : input{input} {}

    void run(TranslationUnit& tu);
};
}  // namespace frontend
