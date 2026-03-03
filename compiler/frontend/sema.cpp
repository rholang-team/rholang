#include "compiler/frontend/sema.hpp"

#include <cassert>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>

#include "compiler/frontend/ast/decl.hpp"
#include "compiler/frontend/ast/declstmt.hpp"
#include "compiler/frontend/ast/expr.hpp"
#include "compiler/frontend/ast/stmt.hpp"
#include "compiler/frontend/ast/visitor.hpp"
#include "compiler/frontend/error.hpp"
#include "compiler/frontend/scopedmap.hpp"
#include "compiler/frontend/translationunit.hpp"
#include "compiler/frontend/type.hpp"

namespace frontend {
namespace {
bool typeIsComparable(const Type& ty) {
    return ty == *PrimitiveType::boolType || ty == *PrimitiveType::intType;
}

class Sema : private ast::DeclVisitor,
             ast::StmtVisitor<void>,
             ast::ExprVisitor<void> {
    using DeclVisitor::visit;
    using StmtVisitor<void>::visit;
    using ExprVisitor<void>::visit;

    ast::File file_;
    ScopedMap<std::string, std::shared_ptr<Type>> scopes_;
    ast::FunctionDecl* curFunction_ = nullptr;
    ast::StructDecl* curStruct_ = nullptr;

    std::optional<std::shared_ptr<StructType>> lookupType(
        const std::string& name) const {
        auto it = file_.structs.find(name);
        if (it == file_.structs.end()) {
            return std::nullopt;
        }

        return std::make_shared<StructType>(it->second.type());
    }

    std::shared_ptr<Type> derefTypeUnchecked(std::shared_ptr<Type> type) const {
        if (const auto* typeRef = dynamic_cast<const TypeRef*>(type.get())) {
            return lookupType(typeRef->name).value();
        }
        return type;
    }

    std::shared_ptr<Type> derefType(
        const lex::WithSpan<std::shared_ptr<Type>>& type) const {
        if (const auto* typeRef = dynamic_cast<TypeRef*>(type.value.get())) {
            auto actualType = lookupType(typeRef->name);
            if (!actualType.has_value()) {
                throw Error(file_.input,
                            type.span,
                            std::format("undefined type {}", typeRef->name));
            }

            return *actualType;
        }
        return type.value;
    }

    void visit(ast::UnaryExpr& expr) {
        visit(expr.value.get());

        switch (expr.op.value) {
            case ast::UnaryExpr::Op::Minus:
                if (*expr.value->type != *PrimitiveType::intType) {
                    throw Error(file_.input,
                                expr.value->span(),
                                "invalid subexpression type");
                }
                break;
            case ast::UnaryExpr::Op::Not:
                if (*expr.value->type != *PrimitiveType::boolType) {
                    throw Error(file_.input,
                                expr.value->span(),
                                "invalid subexpression type");
                }
                break;
        }

        expr.type = expr.value->type;
    }

    void visit(ast::NumLitExpr& expr) {
        expr.type = PrimitiveType::intType;
    }

    void visit(ast::BoolLitExpr& expr) {
        expr.type = PrimitiveType::boolType;
    }

    bool isAssignable(const ast::Expr* expr) {
        bool assignableExpr =
            utils::isa<ast::VarRefExpr>(expr) ||
            (utils::isa<ast::MemberRefExpr>(expr) &&
             isAssignable(
                 dynamic_cast<const ast::MemberRefExpr*>(expr)->target.get()));

        bool assignableType = *expr->type != *PrimitiveType::voidType &&
                              !utils::isa<FunctionType>(expr->type.get());

        return assignableExpr && assignableType;
    }

    void visit(ast::BinaryExpr& expr) {
        visit(expr.lhs.get());
        visit(expr.rhs.get());

        auto checkValidity = [this, &expr](bool cond) {
            if (!cond) {
                throw Error(file_.input,
                            expr.op.span,
                            "invalid types for binary expression");
            }
        };

        std::shared_ptr<Type> lhsType = expr.lhs->type;
        std::shared_ptr<Type> rhsType = expr.rhs->type;

        switch (expr.op.value) {
            case ast::BinaryExpr::Op::Eq:
            case ast::BinaryExpr::Op::Ne:
            case ast::BinaryExpr::Op::Lt:
            case ast::BinaryExpr::Op::Gt:
            case ast::BinaryExpr::Op::Le:
            case ast::BinaryExpr::Op::Ge:
                checkValidity(typeIsComparable(*expr.lhs->type) &&
                              typeIsComparable(*expr.rhs->type) &&
                              *lhsType == *rhsType);
                expr.type = PrimitiveType::boolType;
                break;
            case ast::BinaryExpr::Op::Plus:
            case ast::BinaryExpr::Op::Minus:
            case ast::BinaryExpr::Op::Mul:
                checkValidity(*lhsType == *PrimitiveType::intType &&
                              *rhsType == *PrimitiveType::intType);
                expr.type = PrimitiveType::intType;
                break;
            case ast::BinaryExpr::Op::And:
            case ast::BinaryExpr::Op::Or:
                checkValidity(*lhsType == *PrimitiveType::boolType &&
                              *rhsType == *PrimitiveType::boolType);
                expr.type = PrimitiveType::boolType;
                break;
        }
    }

    void visit(ast::VarRefExpr& expr) {
        auto type = scopes_.lookup(expr.name.value);
        if (!type.has_value()) {
            throw Error(
                file_.input,
                expr.name.span,
                std::format("reference to undefined name {}", expr.name.value));
        }

        expr.type = *type;
    }

    void visit(ast::MemberRefExpr& expr) {
        visit(expr.target.get());

        if (!utils::isa<StructType>(expr.target->type.get())) {
            throw Error(file_.input,
                        expr.target->span(),
                        "value is not a struct");
        }

        StructType& targetStruct =
            dynamic_cast<StructType&>(*expr.target->type);

        auto fieldIt =
            std::ranges::find_if(targetStruct.fields,
                                 [&expr](const StructType::Field& field) {
                                     return field.name == expr.member.value;
                                 });

        if (fieldIt != targetStruct.fields.end()) {
            expr.type = fieldIt->type;
            return;
        }

        auto& methods = file_.structs.at(targetStruct.name).methods;
        auto methodIt = methods.find(expr.member.value);

        if (methodIt != methods.end()) {
            expr.type =
                std::make_shared<FunctionType>(methodIt->second->type());
            return;
        }

        throw Error(
            file_.input,
            expr.member.span,
            std::format("object has no member named {}", expr.member.value));
    }

    void visit(ast::CallExpr& expr) {
        visit(expr.callee.get());

        for (auto& arg : expr.args)
            visit(arg.get());

        FunctionType* fntypePtr =
            dynamic_cast<FunctionType*>(expr.callee->type.get());

        if (!fntypePtr) {
            throw Error(file_.input,
                        expr.callee->span(),
                        std::format("callee of type {} is not callable",
                                    *expr.callee->type));
        }

        FunctionType fntype = *fntypePtr;

        if (auto* memberRef =
                dynamic_cast<ast::MemberRefExpr*>(expr.callee.get())) {
            expr.args.emplace(expr.args.begin(), memberRef->target);

            auto& structDecl = file_.structs.at(
                dynamic_cast<StructType&>(*memberRef->target->type).name);

            auto method = structDecl.methods.at(memberRef->member.value);

            expr.callee =
                std::make_shared<ast::VarRefExpr>(lex::WithSpan<std::string>{
                    method->name.value,
                    expr.callee->span(),
                });

            visit(expr.callee.get());

            fntype = method->type();
        }

        if (expr.args.size() != fntype.params.size()) {
            throw Error(
                file_.input,
                expr.span(),
                std::format(
                    "too {} arguments for call: expected {}, but got {}",
                    (expr.args.size() < fntype.params.size() ? "few" : "many"),
                    fntype.params.size(),
                    expr.args.size()));
        }

        for (const auto& [param, arg] :
             std::ranges::zip_view{fntype.params, expr.args}) {
            if (*param != *arg->type) {
                throw Error(
                    file_.input,
                    arg->span(),
                    std::format(
                        "argument type mismatch: expected {}, but got {}",
                        *param,
                        *arg->type));
            }
        }

        expr.type = fntype.rettype;
    }

    void visit(ast::StructInitExpr& expr) {
        expr.type = derefType(lex::WithSpan{expr.type, expr.tySpan});

        StructType* structType = dynamic_cast<StructType*>(expr.type.get());
        if (!structType) {
            throw Error(file_.input,
                        expr.tySpan,
                        std::format("{} is not a struct type", *expr.type));
        }

        for (const auto& [n, _] : expr.fields) {
            bool nIsExtraField =
                std::ranges::find_if(structType->fields,
                                     [&n](const StructType::Field& f) {
                                         return f.name == n;
                                     }) == structType->fields.end();

            if (nIsExtraField) {
                throw Error(
                    file_.input,
                    expr.span(),
                    std::format("extra field `{}` in struct initializer", n));
            }
        }

        for (const StructType::Field& f : structType->fields) {
            if (!expr.fields.contains(f.name)) {
                throw Error(file_.input,
                            expr.span(),
                            std::format("struct field `{}` is not initialized",
                                        f.name));
            }

            std::shared_ptr<ast::Expr> fieldInitializer =
                expr.fields.at(f.name);
            visit(fieldInitializer.get());

            if (*f.type != *fieldInitializer->type) {
                throw Error(file_.input,
                            fieldInitializer->span(),
                            std::format("field `{}` initializer type mismatch: "
                                        "expected {}, but got {}",
                                        f.name,
                                        *f.type,
                                        *fieldInitializer->type));
            }
        }
    }

    void visit(ast::RetStmt& stmt) {
        std::shared_ptr<Type> rettype = PrimitiveType::voidType;

        if (stmt.value.has_value()) {
            visit(stmt.value->get());
            rettype = (*stmt.value)->type;
        }

        if (*rettype != *curFunction_->rettype.value) {
            throw Error(
                file_.input,
                stmt.span,
                std::format("return type mismatch: expected {}, but got {}",
                            *curFunction_->rettype.value,
                            *rettype));
        }
    }

    void visit(ast::ExprStmt& stmt) {
        visit(stmt.expr.get());
    }

    void visit(ast::AssignmentStmt& stmt) {
        visit(stmt.lhs.get());
        visit(stmt.rhs.get());

        if (!isAssignable(stmt.lhs.get())) {
            throw Error(file_.input,
                        stmt.lhs->span(),
                        "expression is not assignable");
        }
    }

    void visit(ast::DeclStmt& stmt) {
        visit(stmt.decl);
    }

    void visit(ast::CompoundStmt& stmt) {
        scopes_.pushScope();
        for (const auto& s : stmt.stmts) {
            visit(s.get());
        }
        scopes_.popScope();
    }

    void visit(ast::CondStmt& stmt) {
        visit(stmt.cond.get());

        if (*stmt.cond->type != *PrimitiveType::boolType) {
            throw Error(
                file_.input,
                stmt.cond->span(),
                std::format("invalid condition type: expected {}, but got {}",
                            *PrimitiveType::boolType,
                            *stmt.cond->type));
        }

        visit(stmt.onTrue);

        if (stmt.onFalse.has_value()) {
            visit(stmt.onFalse->get());
        }
    }

    void visit(ast::WhileStmt& stmt) {
        visit(stmt.cond.get());

        if (*stmt.cond->type != *PrimitiveType::boolType) {
            throw Error(
                file_.input,
                stmt.cond->span(),
                std::format("invalid condition type: expected {}, but got {}",
                            *PrimitiveType::boolType,
                            *stmt.cond->type));
        }

        visit(stmt.body);
    }

    void visit(ast::VarDecl& decl) {
        auto actualType = derefType(decl.type);

        ast::Expr* value = decl.value.get();
        visit(value);
        if (*value->type != *actualType) {
            throw Error(file_.input,
                        value->span(),
                        std::format("value type does not match declared "
                                    "variable type: expected {}, got {}",
                                    *actualType,
                                    *value->type));
        }

        scopes_.addOrShadow(decl.name.value, actualType);
    }

    void visit(ast::FunctionDecl& decl) {
        if (decl.isInstanceMethod()) {
            if (!curStruct_) {
                throw Error(file_.input,
                            decl.name.span,
                            "methods cannot be created at the top level");
            }

            decl.paramTypes[0].value =
                std::make_shared<StructType>(curStruct_->type());
        }

        curFunction_ = &decl;
        scopes_.addOrShadow(decl.name.value,
                            std::make_shared<FunctionType>(decl.type()));

        scopes_.pushScope();
        for (const auto& [name, type] :
             std::ranges::zip_view{decl.paramNames, decl.paramTypes}) {
            scopes_.addOrShadow(name, derefType(type));
        }
        visit(decl.body);
        scopes_.popScope();
        curFunction_ = nullptr;
    }

    void derefStructMemberTypes() {
        for (auto& [name, s] : file_.structs) {
            for (auto& field : s.fields) {
                field.type.value = derefType(field.type);
            }

            for (auto& [name, decl] : file_.structs.at(name).methods) {
                for (auto& type : decl->paramTypes) {
                    type.value = derefType(type);
                }
                decl->rettype.value = derefType(decl->rettype);
            }
        }
    }

    void derefGlobalTypes() {
        for (auto& [name, decl] : file_.globals) {
            decl.type.value = derefType(decl.type);
        }

        for (auto& [name, decl] : file_.functions) {
            for (const auto& [name, type] :
                 std::ranges::zip_view{decl->paramNames, decl->paramTypes}) {
                type.value = derefType(type);
            }
            decl->rettype.value = derefType(decl->rettype);
        }
    }

    void fillTopLevelScope() {
        scopes_.pushScope();

        for (const auto& [name, decl] : file_.functions) {
            scopes_.addOrShadow(name,
                                std::make_shared<FunctionType>(decl->type()));
        }
        for (const auto& [name, decl] : file_.globals) {
            scopes_.addOrShadow(name, derefType(decl.type));
        }
    }

public:
    Sema(ast::File file) : file_{std::move(file)} {}

    TranslationUnit run() {
        derefStructMemberTypes();
        derefGlobalTypes();
        fillTopLevelScope();

        for (auto& [name, s] : file_.structs) {
            curStruct_ = &s;

            for (auto& field : s.fields) {
                scopes_.addOrShadow(field.name.value, field.type.value);
            }
            for (auto& [methodName, method] : s.methods) {
                scopes_.addOrShadow(
                    methodName,
                    std::make_shared<FunctionType>(method->type()));
            }

            for (auto& [methodName, method] : s.methods) {
                visit(*method);
            }
        }
        curStruct_ = nullptr;

        for (auto& [name, decl] : file_.globals) {
            visit(decl);
        }
        for (auto& [name, decl] : file_.functions) {
            visit(*decl);
        }

        std::unordered_map<std::string, std::shared_ptr<ast::FunctionDecl>>
            functions = file_.functions;
        std::unordered_map<std::string, StructType> structs;

        for (auto& [name, s] : file_.structs) {
            structs.emplace(name, s.type());

            for (auto& [methodName, method] : s.methods) {
                functions.emplace(method->name.value, method);
            }
        }

        return TranslationUnit{
            .globals = std::move(file_.globals),
            .functions = std::move(functions),
            .structs = std::move(structs),
        };
    };
};
}  // namespace

TranslationUnit runSema(ast::File file) {
    return Sema{std::move(file)}.run();
}
}  // namespace frontend
