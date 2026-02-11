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

void Sema::visit(ast::UnaryExpr& expr) {
    visit(expr.value.get());

    switch (expr.op.value) {
        case ast::UnaryExpr::Op::Minus:
            if (!utils::isa<PrimitiveType>(expr.value->type.get()) ||
                dynamic_cast<PrimitiveType*>(expr.value->type.get())->kind !=
                    PrimitiveType::Primitive::Int) {
                throw Error(input, expr.value->span(), "invalid expression type for negation");
            }
    }

    expr.type = expr.value->type;
}

void Sema::visit(ast::NumLitExpr& expr) {
    expr.type = PrimitiveType::intType;
}

void Sema::visit(ast::BinaryExpr& expr) {}

void Sema::visit(ast::VarRefExpr& expr) {}

void Sema::visit(ast::MemberRefExpr& expr) {}

void Sema::visit(ast::CallExpr& expr) {}

void Sema::visit(ast::RetStmt& stmt) {
    std::optional<std::shared_ptr<Type>> rettype;
    if (stmt.value.has_value()) {
        visit(stmt.value->get());
        rettype = (*stmt.value)->type;
    }

    if (**rettype != *curFunction->rettype.value) {
        throw Error(input,
                    curFunction->rettype.span,
                    std::format("return type mismatch: expected {}, but got {}",
                                *curFunction->rettype.value,
                                **rettype));
    }
}

void Sema::visit(ast::DeclStmt& stmt) {
    visit(stmt.decl);
}

void Sema::visit(ast::CompoundStmt& stmt) {
    pushScope();
    for (const auto& s : stmt.stmts) {
        visit(s.get());
    }
    popScope();
}

void Sema::visit(ast::VarDecl& decl) {
    if (decl.value.has_value()) {
        ast::Expr* value = decl.value->get();
        visit(value);
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

void Sema::visit(ast::FunctionDecl& decl) {
    curFunction = &decl;
    addToScope(decl.name.value, std::make_shared<FunctionType>(decl.type()));

    pushScope();
    for (const auto& param : decl.params) {
        addToScope(param.first.value, param.second.value);
    }
    visit(decl.body);
    popScope();
    curFunction = nullptr;
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

    for (const auto& [name, decl] : tu.decls) {
        visit(decl.get());
    }
}
}  // namespace frontend
