#include "compiler/frontend/ast2ir.hpp"

#include "compiler/frontend/ast/visitor.hpp"
#include "compiler/frontend/scopedmap.hpp"
#include "compiler/ir/builder.hpp"

namespace frontend::ast2ir {
namespace {
class Translator : private ast::DeclVisitor,
                   ast::StmtVisitor<void>,
                   ast::ExprVisitor<std::shared_ptr<ir::Value>> {
    using DeclVisitor::visit;
    using StmtVisitor<void>::visit;
    using ExprVisitor<std::shared_ptr<ir::Value>>::visit;

    ScopedMap<std::string, std::shared_ptr<ir::Value>> namedValues_;
    ir::Builder builder_;

    ir::Type* translateType(const Type* ty) {
        if (const PrimitiveType* pty = dynamic_cast<const PrimitiveType*>(ty)) {
            return translateType(*pty);
        } else if (const FunctionType* fty =
                       dynamic_cast<const FunctionType*>(ty)) {
            return translateType(*fty);
        } else if (const StructType* sty =
                       dynamic_cast<const StructType*>(ty)) {
            return translateType(*sty);
        } else {
            throw std::invalid_argument{"unexpected type to translate"};
        }
    }

    ir::Type* translateType(const PrimitiveType& ty) {
        switch (ty.kind) {
            case PrimitiveType::Primitive::Void:
                return builder_.getVoidTy();
            case PrimitiveType::Primitive::Bool:
                return builder_.getBoolTy();
            case PrimitiveType::Primitive::Int:
                return builder_.getIntTy();
        }
    }

    ir::FunctionType* translateType(const FunctionType& ty) {
        std::vector<ir::Type*> translatedParams;
        for (auto& param : ty.params) {
            translatedParams.push_back(translateType(param.get()));
        }

        return builder_.getFunctionTy(translateType(ty.rettype.get()),
                                      translatedParams);
    }

    ir::StructType* translateType(const StructType& ty) {
        std::vector<ir::Type*> translatedFields;
        for (auto& field : ty.fields) {
            translatedFields.push_back(translateType(field.type.get()));
        }

        return builder_.getStructTy(translatedFields);
    }

    void visit(ast::FunctionDecl& decl) {
        ir::Function* res = builder_.startFunction(
            builder_.lookupSignature(decl.name.value).value());

        namedValues_.pushScope();

        for (size_t i = 0; i < decl.paramNames.size(); ++i) {
            namedValues_.addOrShadow(decl.paramNames[i],
                                     builder_.createFnArgRef(res, i));
        }

        for (auto& stmt : decl.body.stmts) {
            visit(stmt.get());
        }

        namedValues_.popScope();
        builder_.finishFunction();
    }

    void visit(ast::VarDecl& decl) {
        auto alloca =
            builder_.createAllocaInstr(translateType(decl.type.value.get()));
        builder_.addToCurBB(alloca);

        namedValues_.addOrShadow(decl.name.value, alloca);

        auto value = visit(decl.value.get());
        builder_.addToCurBB(builder_.createStoreInstr(alloca, value));
    }

    void visit(ast::CompoundStmt& stmt) {}

    void visit(ast::CondStmt& stmt) {}

    void visit(ast::WhileStmt& stmt) {}

    void visit(ast::DeclStmt& stmt) {
        visit(stmt.decl);
    }

    void visit(ast::RetStmt& stmt) {
        builder_.addToCurBB(builder_.createRetInstr(
            stmt.value.transform([this](auto& e) { return visit(e.get()); })));
    }

    void visit(ast::ExprStmt& stmt) {
        visit(stmt.expr.get());
    }

    void visit(ast::AssignmentStmt& stmt) {
        // TODO: somehow get exact target for `store`
    }

    std::shared_ptr<ir::Value> visit(ast::UnaryExpr& expr) {
        auto target = visit(expr.value.get());

        std::shared_ptr<ir::Instr> i;
        switch (expr.op.value) {
            case ast::UnaryExpr::Op::Minus:
                i = builder_.createNegInstr(target);
                break;
            case ast::UnaryExpr::Op::Not:
                i = builder_.createNotInstr(target);
                break;
        }

        builder_.addToCurBB(i);
        return i;
    }

    std::shared_ptr<ir::Value> visit(ast::NumLitExpr& expr) {
        return builder_.createIntImm(expr.value.value);
    }

    std::shared_ptr<ir::Value> visit(ast::BinaryExpr& expr) {
        auto lhs = visit(expr.lhs.get());
        auto rhs = visit(expr.rhs.get());

        std::shared_ptr<ir::Instr> i;
        switch (expr.op.value) {
            case ast::BinaryExpr::Op::Eq:
                i = builder_.createCmpInstr(ir::CmpInstr::Cond::Eq, lhs, rhs);
                break;
            case ast::BinaryExpr::Op::Ne:
                i = builder_.createCmpInstr(ir::CmpInstr::Cond::Ne, lhs, rhs);
                break;
            case ast::BinaryExpr::Op::Lt:
                i = builder_.createCmpInstr(ir::CmpInstr::Cond::Lt, lhs, rhs);
                break;
            case ast::BinaryExpr::Op::Gt:
                i = builder_.createCmpInstr(ir::CmpInstr::Cond::Gt, lhs, rhs);
                break;
            case ast::BinaryExpr::Op::Le:
                i = builder_.createCmpInstr(ir::CmpInstr::Cond::Le, lhs, rhs);
                break;
            case ast::BinaryExpr::Op::Ge:
                i = builder_.createCmpInstr(ir::CmpInstr::Cond::Ge, lhs, rhs);
                break;
            case ast::BinaryExpr::Op::Plus:
                i = builder_.createAddInstr(lhs, rhs);
                break;
            case ast::BinaryExpr::Op::Minus:
                i = builder_.createSubInstr(lhs, rhs);
                break;
            case ast::BinaryExpr::Op::Mul:
                i = builder_.createMulInstr(lhs, rhs);
                break;
            case ast::BinaryExpr::Op::And:
                // TODO
                break;
            case ast::BinaryExpr::Op::Or:
                // TODO
                break;
        }

        builder_.addToCurBB(i);
        return i;
    }

    std::shared_ptr<ir::Value> visit(ast::VarRefExpr& expr) {
        return namedValues_.lookup(expr.name.value).value();
    }

    std::shared_ptr<ir::Value> visit(ast::MemberRefExpr& expr) {
        // TODO: same, somehow get exact target for `gfp`
    }

    std::shared_ptr<ir::Value> visit(ast::CallExpr& expr) {
        ast::VarRefExpr& callee = dynamic_cast<ast::VarRefExpr&>(*expr.callee);

        std::vector<std::shared_ptr<ir::Value>> args;
        for (auto& arg : expr.args) {
            args.emplace_back(visit(arg.get()));
        }

        auto i = builder_.createCallInstr(
            builder_.lookupSignature(callee.name.value).value(),
            std::move(args));
        builder_.addToCurBB(i);

        return i;
    }

    std::shared_ptr<ir::Value> visit(ast::StructInitExpr& expr) {}

public:
    Translator(ir::Context& ctx) : builder_{ctx} {}

    ir::Module run(TranslationUnit& tu) {
        // TODO: translate globals

        for (auto& [_, fn] : tu.functions) {
            builder_.addFunctionSignature(
                std::make_shared<ir::FunctionSignature>(
                    fn->name.value,
                    translateType(fn->type())));
        }

        for (auto& [_, fn] : tu.functions) {
            visit(*fn);
        }

        return builder_.build();
    }
};
}  // namespace

ir::Module translate(ir::Context& ctx, TranslationUnit& tu) {
    return Translator{ctx}.run(tu);
}
}  // namespace frontend::ast2ir
