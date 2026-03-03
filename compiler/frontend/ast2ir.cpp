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

    ast::FunctionDecl* curFunction_;
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

    ir::PointerType* translateType(const StructType& ty) {
        std::vector<ir::Type*> translatedFields;
        for (auto& field : ty.fields) {
            translatedFields.push_back(translateType(field.type.get()));
        }

        return builder_.getPointerTy(builder_.getStructTy(translatedFields));
    }

    std::optional<unsigned> findFieldIdx(const StructType& ty,
                                         const std::string& field) {
        auto it = std::ranges::find(
            ty.fields,
            field,
            [](const StructType::Field& f) -> const std::string& {
                return f.name;
            });

        if (it == ty.fields.end()) {
            return std::nullopt;
        }

        return it - ty.fields.begin();
    }

    std::optional<unsigned> findArgIdx(const std::string& arg) {
        auto it = std::ranges::find(curFunction_->paramNames, arg);

        if (it == curFunction_->paramNames.end()) {
            return std::nullopt;
        }

        return it - curFunction_->paramNames.begin();
    }

    std::shared_ptr<ir::Value> unrollRefToPtr(ast::Expr& e) {
        return unrollRefToPtr(&e);
    }

    std::shared_ptr<ir::Value> unrollRefToPtr(ast::Expr* e) {
        if (ast::VarRefExpr* vre = dynamic_cast<ast::VarRefExpr*>(e)) {
            return namedValues_.lookup(vre->name.value).value();
        } else if (ast::MemberRefExpr* mre =
                       dynamic_cast<ast::MemberRefExpr*>(e)) {
            auto target = unrollRefToPtr(mre->target.get());

            if (utils::isa<ast::MemberRefExpr>(mre->target.get())) {
                target = builder_.addToCurBB(builder_.createLoadInstr(target));
            }

            assert(utils::isa<ir::PointerType>(target->type()));
            assert(!utils::isa<ir::PointerType>(
                dynamic_cast<ir::PointerType*>(target->type())->underlying()));

            unsigned idx =
                findFieldIdx(dynamic_cast<StructType&>(*mre->target->type),
                             mre->member.value)
                    .value();

            auto res = builder_.createGetFieldPtrInstr(target, idx);
            builder_.addToCurBB(res);
            return res;
        } else {
            return visit(e);
        }
    }

    void visit(ast::FunctionDecl& decl) {
        curFunction_ = &decl;

        ir::Function* res = builder_.startFunction(
            builder_.lookupSignature(decl.name.value).value());

        namedValues_.pushScope();

        for (size_t i = 0; i < decl.paramNames.size(); ++i) {
            namedValues_.addOrShadow(decl.paramNames[i],
                                     builder_.createFnArgRef(res, i));
        }

        builder_.startBB();
        visit(decl.body);

        if (builder_.curBasicBlock())
            builder_.finishBB();

        namedValues_.popScope();
        builder_.finishFunction();

        curFunction_ = nullptr;
    }

    void visit(ast::VarDecl& decl) {
        // globals are handled separately

        auto alloca =
            builder_.createAllocaInstr(translateType(decl.type.value.get()));
        builder_.addToCurBB(alloca);

        namedValues_.addOrShadow(decl.name.value, alloca);

        auto value = visit(decl.value.get());
        builder_.addToCurBB(builder_.createStoreInstr(alloca, value));
    }

    void visit(ast::CompoundStmt& stmt) {
        for (auto& s : stmt.stmts) {
            visit(s.get());
        }
    }

    void visit(ast::CondStmt& stmt) {
        auto cond = visit(stmt.cond.get());
        ir::BasicBlock* header = builder_.finishBB();

        ir::BasicBlock* trueBlock = builder_.startBB();
        visit(stmt.onTrue);

        ir::BasicBlock* falseBlock = nullptr;
        if (stmt.onFalse.has_value()) {
            builder_.finishBB();
            falseBlock = builder_.startBB();
            visit(stmt.onFalse->get());
            builder_.finishBB();
        }

        if (!trueBlock->hasTerminator()) {
            ir::BasicBlock* tailBlock = builder_.startBbAndLink();
            trueBlock->addInstr(builder_.createGotoInstr(tailBlock));

            if (falseBlock) {
                if (!falseBlock->hasTerminator())
                    falseBlock->addInstr(builder_.createGotoInstr(tailBlock));
            } else
                falseBlock = tailBlock;
        }

        if (!falseBlock) {
            if (builder_.curBasicBlock() &&
                !builder_.curBasicBlock()->hasTerminator())
                falseBlock = builder_.startBbAndLink();
            else {
                builder_.finishBB();
                falseBlock = builder_.startBB();
            }
        }

        header->addInstr(builder_.createBrInstr(cond, trueBlock, falseBlock));
    }

    void visit(ast::WhileStmt& stmt) {
        ir::BasicBlock* header = builder_.startBbAndLink();
        auto cond = visit(stmt.cond.get());
        builder_.finishBB();

        ir::BasicBlock* body = builder_.startBB();
        visit(stmt.body);
        builder_.finishBB();

        body->addInstr(builder_.createGotoInstr(header));

        ir::BasicBlock* tailBlock = builder_.startBB();
        header->addInstr(builder_.createBrInstr(cond, body, tailBlock));
    }

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
        auto dest = unrollRefToPtr(stmt.lhs.get());
        auto src = visit(stmt.rhs.get());
        builder_.addToCurBB(builder_.createStoreInstr(dest, src));
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

    std::shared_ptr<ir::Value> visit(ast::BoolLitExpr& expr) {
        return builder_.createBoolImm(expr.value.value);
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
        auto val = namedValues_.lookup(expr.name.value).value();

        auto argIdx = findArgIdx(expr.name.value);
        if (argIdx.has_value()) {
            return builder_.createFnArgRef(builder_.curFunction(), *argIdx);
        } else
            return builder_.addToCurBB(builder_.createLoadInstr(val));
    }

    std::shared_ptr<ir::Value> visit(ast::MemberRefExpr& expr) {
        auto src = unrollRefToPtr(expr);

        assert(utils::isa<ir::PointerType>(src->type()));

        auto res = builder_.createLoadInstr(src);
        builder_.addToCurBB(res);
        return res;
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

        assert(i->callee());

        builder_.addToCurBB(i);

        return i;
    }

    std::shared_ptr<ir::Value> visit(ast::StructInitExpr& expr) {
        auto res = builder_.createNewInstr(
            dynamic_cast<ir::PointerType*>(translateType(expr.type.get()))
                ->underlying());

        builder_.addToCurBB(res);

        for (auto& [name, initializer] : expr.fields) {
            auto value = visit(initializer.get());

            auto fieldPtr = builder_.addToCurBB(builder_.createGetFieldPtrInstr(
                res,
                findFieldIdx(dynamic_cast<StructType&>(*expr.type), name)
                    .value()));
            builder_.addToCurBB(builder_.createStoreInstr(fieldPtr, value));
        }

        return res;
    }

public:
    Translator(ir::Context& ctx) : builder_{ctx} {}

    ir::Module run(TranslationUnit& tu) {
        // TODO: translate globals

        for (auto& [_, fn] : tu.functions) {
            builder_.addFunctionSignature(
                std::make_unique<ir::FunctionSignature>(
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
