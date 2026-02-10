#include "frontend/sema.hpp"

#include "frontend/ast/declstmt.hpp"
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

void Sema::run(ast::Stmt* stmt) {
    if (auto* compoundStmt = dynamic_cast<ast::CompoundStmt*>(stmt)) {
        run(*compoundStmt);
    } else if (auto* retStmt = dynamic_cast<ast::RetStmt*>(stmt)) {
        std::optional<std::shared_ptr<Type>> rettype;
        if (retStmt->value.has_value()) {
            run(retStmt->value->get());
            rettype = (*retStmt->value)->type;
        }

        if (**rettype != *curFunction->rettype.value) {
            throw Error(input, curFunction->rettype.span, "return type mismatch"); // TODO: complete message
        }
    } else if (auto* declStmt = dynamic_cast<ast::DeclStmt*>(stmt)) {
        run(declStmt->decl);
    }

    std::unreachable();
}

void Sema::run(ast::CompoundStmt& stmt) {
    pushScope();
    for (const auto& s : stmt.stmts) {
        run(s.get());
    }
    popScope();
}

void Sema::run(ast::VarDecl& decl) {
    if (decl.value.has_value()) {
        ast::Expr* value = decl.value->get();
        run(value);
        if (*value->type != *decl.type.value) {
            throw Error(
                input,
                value->span(),
                std::format("value type does not match declared variable type: expected {}, got {}",
                            *decl.type.value,
                            *value->type));
        }
    }

    addToScope(decl.name.value, decl.type.value);
}

void Sema::run(ast::Decl* decl) {
    if (auto* vardecl = dynamic_cast<ast::VarDecl*>(decl)) {
        run(*vardecl);
    } else if (auto* fndecl = dynamic_cast<ast::FunctionDecl*>(decl)) {
        curFunction = fndecl;
        addToScope(fndecl->name.value, std::make_shared<FunctionType>(fndecl->type()));

        pushScope();
        for (const auto& param : fndecl->params) {
            addToScope(param.first.value, param.second.value);
        }
        run(fndecl->body);
        popScope();
        curFunction = nullptr;
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
