#include "frontend/sema.hpp"

#include <forward_list>
#include <memory>
#include <string>
#include <unordered_map>

#include "frontend/ast/decl.hpp"
#include "frontend/ast/declstmt.hpp"
#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/ast/visitor.hpp"
#include "frontend/error.hpp"
#include "frontend/translationunit.hpp"
#include "frontend/type.hpp"

namespace frontend {
namespace {
bool typeIsComparable(const Type* ty) {
    if (const PrimitiveType* primitive = dynamic_cast<const PrimitiveType*>(ty)) {
        return primitive->kind == PrimitiveType::Primitive::Bool ||
               primitive->kind == PrimitiveType::Primitive::Int;
    }

    return false;
}

class Sema : private ast::DeclVisitor, ast::StmtVisitor<void>, ast::ExprVisitor<void> {
    using DeclVisitor::visit;
    using StmtVisitor<void>::visit;
    using ExprVisitor<void>::visit;

    ast::File file;
    std::unordered_map<std::string, std::shared_ptr<StructType>> structs;
    std::forward_list<std::unordered_map<std::string, std::shared_ptr<Type>>> scopes;
    ast::FunctionDecl* curFunction = nullptr;

    void pushScope() {
        scopes.emplace_front();
    }

    void popScope() {
        scopes.pop_front();
    }

    void addToScope(std::string name, std::shared_ptr<Type> type) {
        scopes.front().insert_or_assign(std::move(name), type);
    }

    std::optional<std::shared_ptr<Type>> lookup(const std::string& name) const {
        for (const auto& scope : scopes) {
            auto it = scope.find(name);
            if (it != scope.end())
                return it->second;
        }

        return std::nullopt;
    }

    std::optional<std::shared_ptr<StructType>> lookupType(const std::string& name) const {
        auto it = structs.find(name);
        if (it == structs.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    std::shared_ptr<Type> derefTypeUnchecked(std::shared_ptr<Type> type) const {
        if (const auto* typeRef = dynamic_cast<const TypeRef*>(type.get())) {
            return lookupType(typeRef->name).value();
        }
        return type;
    }

    std::shared_ptr<Type> derefType(const lex::WithSpan<std::shared_ptr<Type>>& type) const {
        if (const auto* typeRef = dynamic_cast<TypeRef*>(type.value.get())) {
            auto actualType = lookupType(typeRef->name);
            if (!actualType.has_value()) {
                throw Error(file.input, type.span, std::format("undefined type {}", typeRef->name));
            }

            return *actualType;
        }
        return type.value;
    }

    void visit(ast::UnaryExpr& expr) {
        visit(expr.value.get());

        switch (expr.op.value) {
            case ast::UnaryExpr::Op::Minus:
                if (!utils::isa<PrimitiveType>(expr.value->type.get()) ||
                    dynamic_cast<PrimitiveType*>(expr.value->type.get())->kind !=
                        PrimitiveType::Primitive::Int) {
                    throw Error(file.input, expr.value->span(), "invalid subexpression type");
                }
        }

        expr.type = expr.value->type;
    }

    void visit(ast::NumLitExpr& expr) {
        expr.type = PrimitiveType::intType;
    }

    bool isAssignable(const ast::Expr* expr) {
        bool assignableExpr =
            utils::isa<ast::VarRefExpr>(expr) || utils::isa<ast::MemberRefExpr>(expr);

        bool assignableType;
        if (const auto* prim = dynamic_cast<const PrimitiveType*>(expr->type.get())) {
            assignableType = prim->kind != PrimitiveType::Primitive::Void;
        } else {
            assignableType = !utils::isa<FunctionType>(expr->type.get());
        }

        return assignableExpr && assignableType;
    }

    void visit(ast::BinaryExpr& expr) {
        visit(expr.lhs.get());
        visit(expr.rhs.get());

        auto checkValidity = [this, &expr](bool cond) {
            if (!cond) {
                throw Error(file.input, expr.op.span, "invalid types for binary expression");
            }
        };

        std::shared_ptr<Type> lhsType = expr.lhs->type;
        std::shared_ptr<Type> rhsType = expr.rhs->type;

        switch (expr.op.value) {
            case ast::BinaryExpr::Op::Assign:
                checkValidity(isAssignable(expr.lhs.get()) && *lhsType == *rhsType);
                expr.type = expr.rhs->type;
                break;
            case ast::BinaryExpr::Op::Eq:
                checkValidity(typeIsComparable(expr.lhs->type.get()) &&
                              typeIsComparable(expr.rhs->type.get()) && *lhsType == *rhsType);
                expr.type = PrimitiveType::boolType;
                break;
            case ast::BinaryExpr::Op::Plus:
            case ast::BinaryExpr::Op::Minus:
            case ast::BinaryExpr::Op::Mul:
                checkValidity(*lhsType == *PrimitiveType::intType &&
                              *rhsType == *PrimitiveType::intType);
                expr.type = PrimitiveType::intType;
                break;
        }
    }

    void visit(ast::VarRefExpr& expr) {
        auto type = lookup(expr.name.value);
        if (!type.has_value()) {
            throw Error(file.input,
                        expr.name.span,
                        std::format("reference to undefined name {}", expr.name.value));
        }

        expr.type = *type;
    }

    void visit(ast::MemberRefExpr& expr) {
        visit(expr.target.get());

        if (!utils::isa<StructType>(expr.target->type.get())) {
            throw Error(file.input, expr.target->span(), "value is not a struct");
        }

        StructType& targetStruct = dynamic_cast<StructType&>(*expr.target->type);

        auto it = std::ranges::find_if(
            targetStruct.fields,
            [&expr](const StructType::Field& field) { return field.name == expr.member.value; });

        if (it == targetStruct.fields.end()) {
            throw Error(file.input,
                        expr.member.span,
                        std::format("object has no member named {}", expr.member.value));
        }

        expr.type = it->type;
    }

    void visit(ast::CallExpr& expr) {
        // TODO: replace Call(MemberRef(S, M), ...) with Call(mangled(S, M), S, ...)
    }

    void visit(ast::RetStmt& stmt) {
        std::optional<std::shared_ptr<Type>> rettype;
        if (stmt.value.has_value()) {
            visit(stmt.value->get());
            rettype = (*stmt.value)->type;
        }

        if (**rettype != *curFunction->rettype.value) {
            throw Error(file.input,
                        curFunction->rettype.span,
                        std::format("return type mismatch: expected {}, but got {}",
                                    *curFunction->rettype.value,
                                    **rettype));
        }
    }

    void visit(ast::DeclStmt& stmt) {
        visit(stmt.decl);
    }

    void visit(ast::CompoundStmt& stmt) {
        pushScope();
        for (const auto& s : stmt.stmts) {
            visit(s.get());
        }
        popScope();
    }

    void visit(ast::VarDecl& decl) {
        auto actualType = derefType(decl.type);

        if (decl.value.has_value()) {
            ast::Expr* value = decl.value->get();
            visit(value);
            if (*value->type != *actualType) {
                throw Error(
                    file.input,
                    value->span(),
                    std::format(
                        "value type does not match declared variable type: expected {}, got {}",
                        *decl.type.value,
                        *value->type));
            }
        }

        addToScope(decl.name.value, actualType);
    }

    void visit(ast::FunctionDecl& decl) {
        curFunction = &decl;
        addToScope(decl.name.value, std::make_shared<FunctionType>(decl.type()));

        pushScope();
        for (auto& [name, type] : decl.params) {
            addToScope(name.value, derefType(type));
        }
        visit(decl.body);
        popScope();
        curFunction = nullptr;
    }

    void checkAndFillStructs() {
        for (auto& [name, structDecl] : file.structs) {
            std::vector<StructType::Field> fields;
            for (auto& field : structDecl.fields) {
                if (auto* typeRef = dynamic_cast<TypeRef*>(field.type.value.get());
                    typeRef && !file.structs.contains(typeRef->name)) {
                    throw Error(file.input,
                                field.type.span,
                                std::format("undefined type {}", typeRef->name));
                }

                fields.emplace_back(field.name, field.type.value);
            }

            structs.emplace(name, std::make_shared<StructType>(name, std::move(fields)));
        }
    }

    void derefStructFields() {
        for (auto& [name, s] : structs) {
            for (auto& field : s->fields) {
                // types should already be checked during `checkAndFillStructs`
                field.type = derefTypeUnchecked(field.type);
            }
        }
    }

    void derefGlobalTypes() {
        for (auto& [name, decl] : file.globals) {
            decl.type.value = derefType(decl.type);
        }

        for (auto& [name, decl] : file.functions) {
            for (auto& [name, type] : decl.params) {
                type.value = derefType(type);
            }
            decl.rettype.value = derefType(decl.rettype);
        }
    }

    void fillTopLevelScope() {
        pushScope();

        for (const auto& [name, decl] : file.functions) {
            addToScope(name, std::make_shared<FunctionType>(decl.type()));
        }
        for (const auto& [name, decl] : file.globals) {
            addToScope(name, derefType(decl.type));
        }
    }

public:
    Sema(ast::File file) : file{std::move(file)} {}

    TranslationUnit run() {
        checkAndFillStructs();
        derefStructFields();
        derefGlobalTypes();
        fillTopLevelScope();

        for (auto& [name, decl] : file.globals) {
            visit(decl);
        }
        for (auto& [name, decl] : file.functions) {
            visit(decl);
        }

        return TranslationUnit{
            .globals = std::move(file.globals),
            .functions = std::move(file.functions),
            .structs = std::move(structs),
        };
    };
};
}  // namespace

TranslationUnit runSema(ast::File file) {
    return Sema{std::move(file)}.run();
}
}  // namespace frontend
