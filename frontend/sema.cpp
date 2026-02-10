#include "frontend/sema.hpp"

#include "frontend/error.hpp"

namespace frontend {
void Sema::pushScope() {
    scopes.emplace_front();
}

void Sema::popScope() {
    scopes.pop_front();
}

void Sema::addToScope(std::string name, std::shared_ptr<Type> type) {
    scopes.front().insert_or_assign(std::move(name), type);
}

std::optional<std::shared_ptr<Type>> Sema::lookup(const std::string& name) const {
    for (const auto& scope : scopes) {
        auto it = scope.find(name);
        if (it != scope.end())
            return it->second;
    }

    return std::nullopt;
}

void Sema::run(ast::Expr* expr) {}

void Sema::run(ast::Stmt* stmt) {}

void Sema::run(ast::CompoundStmt& stmt) {}

void Sema::run(ast::Decl* decl) {
    if (auto* vardecl = dynamic_cast<ast::VarDecl*>(decl)) {
        if (vardecl->value.has_value()) {
            ast::Expr* value = vardecl->value->get();
            run(value);
            if (*value->type != *vardecl->type.value) {
                throw Error(
                    input,
                    value->span(),
                    std::format(
                        "value type does not match declared variable type: expected {}, got {}",
                        *vardecl->type.value,
                        *value->type));
            }
        }

        addToScope(vardecl->name.value, vardecl->type.value);
    } else if (auto* fndecl = dynamic_cast<ast::FunctionDecl*>(decl)) {
        pushScope();
        for (const auto& param : fndecl->params) {
            addToScope(param.first.value, param.second.value);
        }
        run(fndecl->body);
    }

    std::unreachable();
}

void Sema::run(TranslationUnit& tu) {
    pushScope();
    for (const auto& [name, decl] : tu.decls) {
        if (auto* fndecl = dynamic_cast<ast::FunctionDecl*>(decl.get())) {
            addToScope(name, std::make_shared<FunctionType>(fndecl->type()));
        } else if (auto* vardecl = dynamic_cast<ast::VarDecl*>(decl.get())) {
            addToScope(name, vardecl->type.value);
        }
    }
}
}  // namespace frontend
