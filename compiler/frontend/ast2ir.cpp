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

    TranslationUnit& unit_;
    ast::FunctionDecl* curFunction_;
    ScopedMap<std::string, std::pair<std::shared_ptr<ir::Value>, ir::Type*>>
        namedValues_;
    ir::Builder builder_;

    StructType* derefStructType(Type* ty) {
        StructType* sty = nullptr;
        if (TypeRef* tyref = dynamic_cast<TypeRef*>(ty)) {
            sty = unit_.structs.at(tyref->name).get();
        } else {
            sty = dynamic_cast<StructType*>(ty);
            assert(sty);
        }

        return sty;
    }

    ir::StructType* translateStructType(Type* ty) {
        StructType* sty = derefStructType(ty);

        std::vector<ir::Type*> translatedFields;

        for (const StructType::Field& f : sty->fields) {
            translatedFields.emplace_back(translateType(f.type.get()));
        }

        return builder_.structTy(translatedFields);
    }

    ir::Type* translateType(const Type* ty) {
        if (const PrimitiveType* pty = dynamic_cast<const PrimitiveType*>(ty)) {
            switch (pty->kind) {
                case PrimitiveType::Primitive::Void:
                    return builder_.voidTy();
                case PrimitiveType::Primitive::Bool:
                    return builder_.boolTy();
                case PrimitiveType::Primitive::Int:
                    return builder_.intTy();
            }
        } else if (const FunctionType* fty =
                       dynamic_cast<const FunctionType*>(ty)) {
            return translateType(*fty);
        } else {
            return builder_.pointerTy();
        }
    }

    ir::FunctionType* translateType(const FunctionType& fty) {
        std::vector<ir::Type*> translatedParams;
        for (auto& param : fty.params) {
            translatedParams.push_back(translateType(param.get()));
        }

        return builder_.functionTy(translateType(fty.rettype.get()),
                                   translatedParams);
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

    std::pair<std::shared_ptr<ir::Value>, ir::Type*> unrollRefToPtr(
        ast::Expr& e) {
        return unrollRefToPtr(&e);
    }

    std::pair<std::shared_ptr<ir::Value>, ir::Type*> unrollRefToPtr(
        ast::Expr* e,
        bool topLevel = true) {
        if (ast::VarRefExpr* vre = dynamic_cast<ast::VarRefExpr*>(e)) {
            if (topLevel)
                return namedValues_.lookup(vre->name.value).value();
            else {
                auto [ptr, ptrTy] =
                    namedValues_.lookup(vre->name.value).value().get();
                return {builder_.addToCurBb(builder_.loadInstr(ptrTy, ptr)), ptrTy};
            }
        } else if (ast::MemberRefExpr* mre =
                       dynamic_cast<ast::MemberRefExpr*>(e)) {
            auto [target, targetTy] =
                unrollRefToPtr(mre->target.get(), /* topLevel= */ false);

            if (utils::isa<ast::MemberRefExpr>(mre->target.get())) {
                target =
                    builder_.addToCurBb(builder_.loadInstr(targetTy, target));
            }

            assert(utils::isa<ir::PointerType>(target->type()));

            ir::StructType* sty = translateStructType(mre->target->type.get());
            unsigned idx =
                findFieldIdx(
                    *unit_.structs.at(
                        dynamic_cast<TypeRef&>(*mre->target->type).name),
                    mre->member.value)
                    .value();

            auto res = builder_.getFieldPtrInstr(sty, target, idx);
            builder_.addToCurBb(res);
            return {res, res->fieldType()};
        } else {
            return {visit(e), translateType(e->type.get())};
        }
    }

    void visit(ast::FunctionDecl& decl) {
        curFunction_ = &decl;

        ir::Function* res = builder_.startFunction(
            builder_.lookupSignature(decl.name.value).value());

        namedValues_.pushScope();

        builder_.startBb();

        for (size_t i = 0; i < decl.paramNames.size(); ++i) {
            auto ty = res->signature()->type()->params()[i];
            auto alloca = builder_.allocaInstr(ty);
            builder_.addToCurBb(alloca);
            builder_.addToCurBb(
                builder_.storeInstr(ty,
                                    alloca,
                                    builder_.fnArgRef(res->signature(), i)));

            namedValues_.addOrShadow(decl.paramNames[i], std::pair{alloca, ty});
        }

        builder_.startBbAndLink();
        visit(decl.body);

        if (builder_.curBb()) {
            if (!builder_.curBb()->hasTerminator() &&
                res->signature()->type()->rettype() == builder_.voidTy()) {
                builder_.curBb()->addInstr(builder_.retInstr());
            }
            builder_.finishBb();
        }

        namedValues_.popScope();
        builder_.finishFunction();

        curFunction_ = nullptr;
    }

    void visit(ast::VarDecl& decl) {
        // globals are handled separately

        ir::Type* ty = translateType(decl.type.value.get());
        auto alloca = builder_.allocaInstr(ty);
        builder_.addToCurBb(alloca);

        namedValues_.addOrShadow(decl.name.value, std::pair{alloca, ty});

        auto value = visit(decl.value.get());
        builder_.addToCurBb(
            builder_.storeInstr(translateType(decl.type.value.get()),
                                alloca,
                                value));
    }

    void visit(ast::CompoundStmt& stmt) {
        for (auto& s : stmt.stmts) {
            visit(s.get());
        }
    }

    void visit(ast::CondStmt& stmt) {
        auto cond = visit(stmt.cond.get());
        ir::BasicBlock* header = builder_.finishBb();

        ir::BasicBlock* trueBlock = builder_.startBb();
        visit(stmt.onTrue);

        ir::BasicBlock* innerTailBlock = nullptr;
        ir::BasicBlock* falseBlock = nullptr;
        if (stmt.onFalse.has_value()) {
            builder_.finishBb();
            falseBlock = builder_.startBb();
            visit(stmt.onFalse->get());
            innerTailBlock = builder_.finishBb();
        }

        if (!trueBlock->hasTerminator()) {
            ir::BasicBlock* tailBlock = builder_.startBbAndLink();
            trueBlock->addInstr(builder_.gotoInstr(tailBlock));
            innerTailBlock->addInstr(builder_.gotoInstr(tailBlock));

            if (falseBlock) {
                if (!falseBlock->hasTerminator())
                    falseBlock->addInstr(builder_.gotoInstr(tailBlock));
            } else
                falseBlock = tailBlock;
        }

        if (!falseBlock) {
            if (builder_.curBb() && !builder_.curBb()->hasTerminator())
                falseBlock = builder_.startBbAndLink();
            else {
                builder_.finishBb();
                falseBlock = builder_.startBb();
            }
        }

        header->addInstr(builder_.brInstr(cond, trueBlock, falseBlock));
    }

    void visit(ast::WhileStmt& stmt) {
        ir::BasicBlock* headerStart = builder_.startBbAndLink();
        auto cond = visit(stmt.cond.get());
        ir::BasicBlock* headerEnd = builder_.finishBb();

        ir::BasicBlock* body = builder_.startBb();
        visit(stmt.body);

        builder_.addToCurBb(builder_.gotoInstr(headerStart, /*backedge=*/true));
        builder_.finishBb();

        ir::BasicBlock* tailBlock = builder_.startBb();
        headerEnd->addInstr(builder_.brInstr(cond, body, tailBlock));
    }

    void visit(ast::DeclStmt& stmt) {
        visit(stmt.decl);
    }

    void visit(ast::RetStmt& stmt) {
        builder_.addToCurBb(builder_.retInstr(
            stmt.value.transform([this](auto& e) { return visit(e.get()); })));
    }

    void visit(ast::ExprStmt& stmt) {
        visit(stmt.expr.get());
    }

    void visit(ast::AssignmentStmt& stmt) {
        auto [dest, destTy] = unrollRefToPtr(stmt.lhs.get());
        auto src = visit(stmt.rhs.get());
        builder_.addToCurBb(builder_.storeInstr(destTy, dest, src));
    }

    std::shared_ptr<ir::Value> visit(ast::UnaryExpr& expr) {
        auto target = visit(expr.value.get());

        std::shared_ptr<ir::Instr> i;
        switch (expr.op.value) {
            case ast::UnaryExpr::Op::Minus:
                i = builder_.negInstr(target);
                break;
            case ast::UnaryExpr::Op::Not:
                i = builder_.notInstr(target);
                break;
        }

        builder_.addToCurBb(i);
        return i;
    }

    std::shared_ptr<ir::Value> visit(ast::NumLitExpr& expr) {
        return builder_.intImm(expr.value.value);
    }

    std::shared_ptr<ir::Value> visit(ast::BoolLitExpr& expr) {
        return builder_.boolImm(expr.value.value);
    }

    std::shared_ptr<ir::Value> visit(ast::BinaryExpr& expr) {
        auto lhs = visit(expr.lhs.get());

        std::shared_ptr<ir::Instr> res;
        switch (expr.op.value) {
            case ast::BinaryExpr::Op::Eq: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.cmpInstr(ir::CmpInstr::Cond::Eq, lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Ne: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.cmpInstr(ir::CmpInstr::Cond::Ne, lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Lt: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.cmpInstr(ir::CmpInstr::Cond::Lt, lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Gt: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.cmpInstr(ir::CmpInstr::Cond::Gt, lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Le: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.cmpInstr(ir::CmpInstr::Cond::Le, lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Ge: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.cmpInstr(ir::CmpInstr::Cond::Ge, lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Plus: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.addInstr(lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Minus: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.subInstr(lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Mul: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.mulInstr(lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::Div: {
                auto rhs = visit(expr.rhs.get());
                res = builder_.divInstr(lhs, rhs);
                builder_.addToCurBb(res);
                break;
            }
            case ast::BinaryExpr::Op::And: {
                res = builder_.addToCurBb(
                    builder_.allocaInstr(builder_.boolTy()));
                ir::BasicBlock* lhsBlock = builder_.finishBb();

                ir::BasicBlock* trueBlock = builder_.startBb();
                auto rhs = visit(expr.rhs.get());
                builder_.addToCurBb(
                    builder_.storeInstr(builder_.boolTy(), res, rhs));
                builder_.finishBb();

                ir::BasicBlock* falseBlock = builder_.startBb();
                builder_.addToCurBb(
                    builder_.storeInstr(builder_.boolTy(),
                                        res,
                                        builder_.boolImm(false)));

                ir::BasicBlock* nextBlock = builder_.startBbAndLink();

                trueBlock->addInstr(builder_.gotoInstr(nextBlock));
                lhsBlock->addInstr(
                    builder_.brInstr(lhs, trueBlock, falseBlock));
                break;
            }
            case ast::BinaryExpr::Op::Or: {
                res = builder_.addToCurBb(
                    builder_.allocaInstr(builder_.boolTy()));
                ir::BasicBlock* lhsBlock = builder_.finishBb();

                ir::BasicBlock* trueBlock = builder_.startBb();
                builder_.addToCurBb(
                    builder_.storeInstr(builder_.boolTy(),
                                        res,
                                        builder_.boolImm(true)));
                builder_.finishBb();

                ir::BasicBlock* falseBlock = builder_.startBb();
                auto rhs = visit(expr.rhs.get());
                builder_.addToCurBb(
                    builder_.storeInstr(builder_.boolTy(), res, rhs));

                ir::BasicBlock* nextBlock = builder_.startBbAndLink();

                trueBlock->addInstr(builder_.gotoInstr(nextBlock));
                lhsBlock->addInstr(
                    builder_.brInstr(lhs, trueBlock, falseBlock));
                break;
            }
        }

        return res;
    }

    std::shared_ptr<ir::Value> visit(ast::VarRefExpr& expr) {
        // auto argIdx = findArgIdx(expr.name.value);
        // if (argIdx.has_value()) {
        //     return builder_.fnArgRef(builder_.curFunction()->signature(),
        //                              *argIdx);
        // } else {
        auto& [ptr, valTy] = namedValues_.lookup(expr.name.value).value().get();
        return builder_.addToCurBb(builder_.loadInstr(valTy, ptr));
        // }
    }

    std::shared_ptr<ir::Value> visit(ast::MemberRefExpr& expr) {
        auto [src, valTy] = unrollRefToPtr(expr);

        assert(utils::isa<ir::PointerType>(src->type()));

        auto res = builder_.loadInstr(valTy, src);
        builder_.addToCurBb(res);
        return res;
    }

    std::shared_ptr<ir::Value> visit(ast::NullExpr&) {
        return builder_.nullPtr();
    }

    std::shared_ptr<ir::Value> visit(ast::CallExpr& expr) {
        ast::VarRefExpr& callee = dynamic_cast<ast::VarRefExpr&>(*expr.callee);

        std::vector<std::shared_ptr<ir::Value>> args;
        for (auto& arg : expr.args) {
            args.emplace_back(visit(arg.get()));
        }

        auto i = builder_.callInstr(
            builder_.lookupSignature(callee.name.value).value(),
            std::move(args));

        assert(i->callee());

        builder_.addToCurBb(i);

        return i;
    }

    std::shared_ptr<ir::Value> visit(ast::StructInitExpr& expr) {
        ir::StructType* sty = translateStructType(expr.type.get());
        auto res = builder_.newInstr(sty);

        builder_.addToCurBb(res);

        for (auto& [name, initializer] : expr.fields) {
            auto value = visit(initializer.get());

            auto fieldIdx =
                findFieldIdx(*derefStructType(expr.type.get()), name).value();
            auto gfp = builder_.getFieldPtrInstr(sty, res, fieldIdx);

            builder_.addToCurBb(gfp);
            builder_.addToCurBb(
                builder_.storeInstr(gfp->fieldType(), gfp, value));
        }

        return res;
    }

public:
    Translator(ir::Context& ctx, TranslationUnit& tu)
        : unit_{tu}, builder_{ctx} {}

    ir::Module run() {
        for (auto& [_, fn] : unit_.functions) {
            builder_.addFunctionSignature(fn->name.value,
                                          translateType(fn->type()));
        }

        if (!unit_.globals.empty()) {
            builder_.startFunction(
                "_Rglobal_init",
                builder_.functionTy(builder_.voidTy(), std::span<ir::Type*>{}));
            builder_.startBb();

            namedValues_.pushScope();
            for (auto& [_, global] : unit_.globals) {
                ir::Type* ty = translateType(global.type.value.get());
                auto globalPtr = builder_.addGlobal(global.name.value, ty);

                namedValues_.addOrShadow(global.name.value,
                                         std::pair{globalPtr, ty});

                auto value = visit(global.value.get());
                builder_.addToCurBb(
                    builder_.storeInstr(translateType(global.type.value.get()),
                                        globalPtr,
                                        value));
            }

            builder_.addToCurBb(builder_.retInstr());
            builder_.finishBb();
            builder_.finishFunction();
        }

        for (auto& [_, fn] : unit_.functions) {
            visit(*fn);
        }

        return builder_.build();
    }
};
}  // namespace

ir::Module translate(ir::Context& ctx, TranslationUnit& tu) {
    return Translator{ctx, tu}.run();
}
}  // namespace frontend::ast2ir
