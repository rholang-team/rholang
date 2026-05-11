#include "compiler/backend/lowerir.hpp"

#include <algorithm>
#include <print>
#include <ranges>

#include "compiler/ir/visitor.hpp"
#include "compiler/lir/instr.hpp"
#include "compiler/lir/register.hpp"
#include "compiler/lir/value.hpp"

namespace {
size_t valueAlignment(const ir::Type* ty) {
    assert(!utils::isa<ir::VoidType>(ty));
    assert(!utils::isa<ir::FunctionType>(ty));

    if (utils::isa<ir::IntType>(ty)) {
        return 4;
    } else if (utils::isa<ir::PointerType>(ty)) {
        return 8;
    } else if (utils::isa<ir::BoolType>(ty)) {
        return 1;
    } else if (utils::isa<ir::StructType>(ty)) {
        const ir::StructType* sty = static_cast<const ir::StructType*>(ty);
        size_t res = 0;
        for (const ir::Type* fieldTy : sty->fields()) {
            res = std::max(res, valueAlignment(fieldTy));
        }
        return res;
    }

    std::unreachable();
}

size_t valueSize(const ir::Type* ty);

template <std::ranges::range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, const ir::Type*>
size_t totalSizeWithAlignment(R&& r) {
    size_t res = 0;
    for (const ir::Type* itemTy : r) {
        size_t size = valueSize(itemTy);
        size_t alignment = valueAlignment(itemTy);
        size_t alignmentFix =
            (res % alignment == 0) ? 0 : alignment - res % alignment;
        res += alignmentFix + size;
    }
    return res;
}

size_t valueSize(const ir::Type* ty) {
    assert(!utils::isa<ir::VoidType>(ty));
    assert(!utils::isa<ir::FunctionType>(ty));

    if (utils::isa<ir::IntType>(ty)) {
        return 4;
    } else if (utils::isa<ir::PointerType>(ty)) {
        return 8;
    } else if (utils::isa<ir::BoolType>(ty)) {
        return 1;
    } else if (utils::isa<ir::StructType>(ty)) {
        const ir::StructType* sty = static_cast<const ir::StructType*>(ty);
        return totalSizeWithAlignment(sty->fields());
    }

    std::unreachable();
}

lir::WordType valueWordType(const ir::Type* ty) {
    assert(!utils::isa<ir::VoidType>(ty));
    assert(!utils::isa<ir::FunctionType>(ty));
    assert(!utils::isa<ir::StructType>(ty));

    if (utils::isa<ir::IntType>(ty)) {
        return lir::WordType::Dword;
    } else if (utils::isa<ir::PointerType>(ty)) {
        return lir::WordType::Qword;
    } else if (utils::isa<ir::BoolType>(ty)) {
        return lir::WordType::Byte;
    }

    std::unreachable();
}

size_t fieldOffset(const ir::StructType* structTy, size_t fieldIdx) {
    return totalSizeWithAlignment(
        std::ranges::take_view(structTy->fields(), fieldIdx));
}

class Lowerer final : public ir::Visitor<void, true> {
    using Super = ir::Visitor<void, true>;
    using Super::visit;
    using Super::visitInstr;

    lir::Module res_;
    std::optional<lir::Function> fn_;
    lir::BasicBlock* bb_;

    std::unordered_map<const ir::Value*, std::shared_ptr<lir::VirtualRegister>>
        virtualRegisterMap_;

    std::vector<std::shared_ptr<lir::VirtualRegister>> fnArgs_;

    std::unordered_map<const ir::Value*, std::shared_ptr<lir::VirtualRegister>>
        fakedAllocas_;

    std::vector<bool> frameMap_;

    std::unordered_map<const ir::BasicBlock*, lir::BasicBlock*> bbMap_;

    lir::VirtualRegisterFactory vregFactory_;

    std::shared_ptr<lir::VirtualRegister> newVirtualRegister(
        const ir::Value& valuePut) {
        auto res =
            newVirtualRegister(utils::isa<ir::PointerType>(valuePut.type()));
        virtualRegisterMap_.emplace(&valuePut, res);
        return res;
    }

    std::shared_ptr<lir::VirtualRegister> newVirtualRegister(bool isPtr) {
        frameMap_.push_back(isPtr);
        return vregFactory_.nextShared();
    }

    std::shared_ptr<lir::Register> translateInstrArgument(const ir::Value& v) {
        if (auto it = virtualRegisterMap_.find(&v);
            it != virtualRegisterMap_.end()) {
            return it->second;
        }

        if (const ir::IntImm* intImm = dynamic_cast<const ir::IntImm*>(&v)) {
            auto reg = newVirtualRegister(v);
            bb_->addInstr(
                std::make_unique<lir::LoadImmInstr>(reg, intImm->value()));
            return reg;
        } else if (const ir::BoolImm* boolImm =
                       dynamic_cast<const ir::BoolImm*>(&v)) {
            auto reg = newVirtualRegister(v);
            bb_->addInstr(
                std::make_unique<lir::LoadImmInstr>(reg, boolImm->value()));
            return reg;
        } else if (utils::isa<ir::NullPtr>(&v)) {
            auto reg = newVirtualRegister(v);
            bb_->addInstr(std::make_unique<lir::LoadImmInstr>(reg, 0));
            return reg;
        } else if (const ir::FnArgRef* fnArg =
                       dynamic_cast<const ir::FnArgRef*>(&v)) {
            auto reg = fnArgs_[fnArg->idx()];
            virtualRegisterMap_.emplace(&v, reg);
            return reg;
        } else if (const ir::GlobalPtr* global =
                       dynamic_cast<const ir::GlobalPtr*>(&v)) {
            auto reg = newVirtualRegister(false);
            bb_->addInstr(std::make_unique<lir::LeaInstr>(
                reg,
                std::make_shared<lir::GlobalRef>(global->name())));
            virtualRegisterMap_.emplace(&v, reg);
            return reg;
        }

        std::unreachable();
    }

    void translateInstrArgumentIntoReg(
        const ir::Value& v,
        std::shared_ptr<lir::Register> desiredReg) {
        if (auto it = virtualRegisterMap_.find(&v);
            it != virtualRegisterMap_.end()) {
            bb_->addInstr(
                std::make_unique<lir::MovInstr>(desiredReg, it->second));
            return;
        }

        if (const ir::IntImm* intImm = dynamic_cast<const ir::IntImm*>(&v)) {
            bb_->addInstr(std::make_unique<lir::LoadImmInstr>(desiredReg,
                                                              intImm->value()));
        } else if (const ir::BoolImm* boolImm =
                       dynamic_cast<const ir::BoolImm*>(&v)) {
            bb_->addInstr(
                std::make_unique<lir::LoadImmInstr>(desiredReg,
                                                    boolImm->value()));
        } else if (utils::isa<ir::NullPtr>(&v)) {
            bb_->addInstr(std::make_unique<lir::LoadImmInstr>(desiredReg, 0));
        } else if (const ir::FnArgRef* fnArg =
                       dynamic_cast<const ir::FnArgRef*>(&v)) {
            bb_->addInstr(
                std::make_unique<lir::MovInstr>(desiredReg,
                                                fnArgs_[fnArg->idx()]));
        } else {
            std::println(stderr, "match error on `{}`", typeid(v).name());
            std::unreachable();
        }
    }

    void visit(const ir::Module& module) override {
        for (auto&& [_, fn] : module.functions()) {
            visit(*fn);
            res_.addFunction(*std::move(fn_));
            fn_.reset();
        }

        for (auto&& [_, global] : module.globals()) {
            visitGlobalDecl(*global);
        }
    }

    void visitGlobalDecl(const ir::GlobalPtr& global) override {
        res_.addGlobal(global.name(), valueWordType(global.valueTy()));
    }

    lir::BasicBlock* translateBb(const ir::BasicBlock* bb) const {
        return bbMap_.at(bb);
    }

    void addArgLoads(const ir::FunctionSignature& sig,
                     lir::BasicBlock& prologue,
                     size_t vregCount) {
        auto params = sig.type()->params();

        const auto& argumentRegisters = lir::Function::argumentRegisters;

        for (size_t i = 0; i < params.size(); ++i) {
            if (i < argumentRegisters.size()) {
                prologue.addInstr(std::make_unique<lir::MovInstr>(
                    fnArgs_[i],
                    lir::PhysicalRegister::create(argumentRegisters[i])));
                continue;
            }

            int disp = lir::wordTypeToSize(lir::WordType::Qword) *
                       (vregCount + i - argumentRegisters.size() + 1);

            prologue.addInstr(std::make_unique<lir::LoadInstr>(
                valueWordType(params[i]),
                fnArgs_[i],
                std::make_shared<lir::AddressExpression>(
                    lir::StackPointer::create(),
                    disp)));
        }
    }

    void visit(const ir::Function& fn) override {
        virtualRegisterMap_.clear();
        fakedAllocas_.clear();
        bbMap_.clear();
        fnArgs_.clear();
        vregFactory_.reset();
        frameMap_.clear();

        fn_ = lir::Function(fn.signature()->name(),
                            fn.signature()->type()->params().size(),
                            !fn.signature()->type()->rettype()->isVoid());

        size_t bbIdx = 0;

        auto prologueUnique = std::make_unique<lir::BasicBlock>(bbIdx++);
        lir::BasicBlock* prologue = prologueUnique.get();
        for (size_t i = 0; i < fn.signature()->type()->params().size(); ++i) {
            fnArgs_.push_back(newVirtualRegister(utils::isa<ir::PointerType>(
                fn.signature()->type()->params()[i])));
        }
        fn_->addBb(std::move(prologueUnique));

        for (const auto& bb : fn) {
            auto lBb = std::make_unique<lir::BasicBlock>(bbIdx++);
            bbMap_.emplace(bb.get(), lBb.get());
            fn_->addBb(std::move(lBb));
        }

        for (const auto& bb : fn) {
            bb_ = translateBb(bb.get());
            visit(*bb);
        }

        auto framemapVReg = newVirtualRegister(false);
        auto vregCount = vregFactory_.created();
        prologue->addInstr(std::make_unique<lir::SubInstr>(
            lir::StackPointer::create(),
            lir::Immediate::create(vregCount *
                                   lir::wordTypeToSize(lir::WordType::Qword))));

        for (size_t i = 0; i < frameMap_.size(); ++i) {
            bool notArg = std::ranges::find_if(
                              fnArgs_,
                              [i](std::shared_ptr<lir::VirtualRegister> r) {
                                  return r->id() == i;
                              }) == fnArgs_.end();
            if (frameMap_[i] && notArg) {
                prologue->addInstr(std::make_unique<lir::LoadImmInstr>(
                    lir::VirtualRegister::create(i),
                    0));
            }
        }
        fn_->setFrameMap(std::move(frameMap_));

        addArgLoads(*fn.signature(), *prologue, vregCount);
        prologue->addInstr(std::make_unique<lir::FrameEntryInstr>());
        prologue->addInstr(std::make_unique<lir::SafePointInstr>());
    }

    void visitAllocaInstr(const ir::AllocaInstr& i) override {
        fakedAllocas_.emplace(
            &i,
            newVirtualRegister(utils::isa<ir::PointerType>(i.itemType())));
    }
    template <std::ranges::range R>
        requires std::convertible_to<
            std::ranges::range_value_t<std::remove_cvref_t<R>>,
            std::shared_ptr<lir::Register>>
    void makeAbiCompliantCall(
        lir::BasicBlock& bb,
        std::string_view callee,
        R&& args,
        std::optional<std::shared_ptr<lir::Register>> ret = std::nullopt) {
        const auto& argumentRegisters = lir::Function::argumentRegisters;

        for (size_t i = 0; i < args.size(); ++i) {
            if (i < argumentRegisters.size()) {
                bb.addInstr(std::make_unique<lir::MovInstr>(
                    lir::PhysicalRegister::create(argumentRegisters[i]),
                    args[i]));
                continue;
            }

            auto arg = args[args.size() - (i - argumentRegisters.size()) - 1];
            bb.addInstr(std::make_unique<lir::PushInstr>(arg));
        }

        bb.addInstr(std::make_unique<lir::CallInstr>(callee));

        if (args.size() > argumentRegisters.size()) {
            bb.addInstr(std::make_unique<lir::AddInstr>(
                lir::StackPointer::create(),
                lir::Immediate::create(
                    lir::wordTypeToSize(lir::WordType::Qword) *
                    (args.size() - argumentRegisters.size()))));
        }

        if (ret.has_value()) {
            bb.addInstr(std::make_unique<lir::MovInstr>(
                *ret,
                lir::PhysicalRegister::create(
                    lir::PhysicalRegister::Name::Rax)));
        }
    }

    void visitNewInstr(const ir::NewInstr& i) override {
        // TODO: add object map pointer to args
        // probably in form of a global ref

        auto sizeReg = newVirtualRegister(false);
        bb_->addInstr(
            std::make_unique<lir::LoadImmInstr>(sizeReg,
                                                valueSize(i.itemType())));
        std::vector<std::shared_ptr<lir::Register>> args{sizeReg};

        auto dest = newVirtualRegister(i);

        makeAbiCompliantCall(*bb_, "runtime_alloc", std::move(args), dest);
    }

    void visitCallInstr(const ir::CallInstr& i) override {
        std::vector<std::shared_ptr<lir::Register>> args;
        for (auto&& arg : i.args()) {
            args.emplace_back(translateInstrArgument(*arg));
        }

        bool returnsValue = !i.type()->isVoid();
        makeAbiCompliantCall(*bb_,
                             i.callee()->name(),
                             std::move(args),
                             (returnsValue)
                                 ? std::optional{newVirtualRegister(i)}
                                 : std::nullopt);
    }

    void visitNotInstr(const ir::NotInstr& i) override {
        bb_->addInstr(std::make_unique<lir::NotInstr>(
            translateInstrArgument(*i.target())));
    }

    void visitNegInstr(const ir::NegInstr& i) override {
        bb_->addInstr(std::make_unique<lir::NegInstr>(
            translateInstrArgument(*i.target())));
    }

    void visitLoadInstr(const ir::LoadInstr& i) override {
        if (auto it = fakedAllocas_.find(i.src().get());
            it != fakedAllocas_.end()) {
            virtualRegisterMap_.emplace(&i, it->second);
            return;
        }

        bb_->addInstr(std::make_unique<lir::LoadInstr>(
            valueWordType(i.type()),
            newVirtualRegister(i),
            std::dynamic_pointer_cast<lir::Address>(
                translateInstrArgument(*i.src()))));
    }

    void visitStoreInstr(const ir::StoreInstr& i) override {
        if (auto it = fakedAllocas_.find(i.dest().get());
            it != fakedAllocas_.end()) {
            translateInstrArgumentIntoReg(*i.src(), it->second);
            return;
        }

        bb_->addInstr(std::make_unique<lir::StoreInstr>(
            valueWordType(i.storedValueType()),
            std::dynamic_pointer_cast<lir::Address>(
                translateInstrArgument(*i.dest())),
            translateInstrArgument(*i.src())));
    }

    void visitAddInstr(const ir::AddInstr& i) override {
        auto lhs = translateInstrArgument(*i.lhs());

        auto lhsDest = newVirtualRegister(i);
        bb_->addInstr(std::make_unique<lir::MovInstr>(lhsDest, lhs));

        bb_->addInstr(
            std::make_unique<lir::AddInstr>(lhsDest,
                                            translateInstrArgument(*i.rhs())));
    }
    void visitSubInstr(const ir::SubInstr& i) override {
        auto lhs = translateInstrArgument(*i.lhs());

        auto lhsDest = newVirtualRegister(i);
        bb_->addInstr(std::make_unique<lir::MovInstr>(lhsDest, lhs));

        bb_->addInstr(
            std::make_unique<lir::SubInstr>(lhsDest,
                                            translateInstrArgument(*i.rhs())));
    }
    void visitMulInstr(const ir::MulInstr& i) override {
        auto lhs = translateInstrArgument(*i.lhs());

        auto lhsDest = newVirtualRegister(i);
        bb_->addInstr(std::make_unique<lir::MovInstr>(lhsDest, lhs));

        bb_->addInstr(
            std::make_unique<lir::MulInstr>(lhsDest,
                                            translateInstrArgument(*i.rhs())));
    }
    void visitDivInstr(const ir::DivInstr& i) override {
        // TODO: `cdq` in `idiv` emission

        bb_->addInstr(std::make_unique<lir::MovInstr>(
            lir::PhysicalRegister::create(lir::PhysicalRegister::Name::Rax),
            translateInstrArgument(*i.lhs())));
        bb_->addInstr(
            std::make_unique<lir::DivInstr>(translateInstrArgument(*i.rhs())));
        bb_->addInstr(std::make_unique<lir::MovInstr>(
            newVirtualRegister(i),
            lir::PhysicalRegister::create(lir::PhysicalRegister::Name::Rax)));
    }

    void visitCmpInstr(const ir::CmpInstr& i) override {
        lir::CmpInstr::Cond cond;
        switch (i.cond()) {
            case ir::CmpInstr::Cond::Eq:
                cond = lir::CmpInstr::Cond::Eq;
                break;
            case ir::CmpInstr::Cond::Ne:
                cond = lir::CmpInstr::Cond::Ne;
                break;
            case ir::CmpInstr::Cond::Lt:
                cond = lir::CmpInstr::Cond::Lt;
                break;
            case ir::CmpInstr::Cond::Gt:
                cond = lir::CmpInstr::Cond::Gt;
                break;
            case ir::CmpInstr::Cond::Le:
                cond = lir::CmpInstr::Cond::Le;
                break;
            case ir::CmpInstr::Cond::Ge:
                cond = lir::CmpInstr::Cond::Ge;
                break;
        }

        bb_->addInstr(
            std::make_unique<lir::CmpInstr>(valueWordType(i.lhs()->type()),
                                            newVirtualRegister(i),
                                            cond,
                                            translateInstrArgument(*i.lhs()),
                                            translateInstrArgument(*i.rhs())));
    }

    void visitGetFieldPtrInstr(const ir::GetFieldPtrInstr& i) override {
        std::shared_ptr<lir::Register> target =
            std::dynamic_pointer_cast<lir::Register>(
                translateInstrArgument(*i.target()));

        size_t offset = fieldOffset(i.structType(), i.fieldIdx());

        bb_->addInstr(std::make_unique<lir::LeaInstr>(
            newVirtualRegister(i),
            std::make_shared<lir::AddressExpression>(target, offset)));
    }

    void visitGotoInstr(const ir::GotoInstr& i) override {
        if (i.containsBackedge()) {
            bb_->addInstr(std::make_unique<lir::SafePointInstr>());
        }
        bb_->addInstr(std::make_unique<lir::JmpInstr>(translateBb(i.dest())));
    }

    void visitBrInstr(const ir::BrInstr& i) override {
        lir::BasicBlock* dest;
        std::pair<std::shared_ptr<lir::Register>, bool> cond;

        lir::BasicBlock* onTrue = translateBb(i.onTrue());
        lir::BasicBlock* onFalse = translateBb(i.onFalse());

        {
            auto curBb = std::ranges::find_if(*fn_, [this](const auto& bb) {
                return bb.get() == bb_;
            });
            assert(curBb != fn_->end());

            auto nextBb = ++curBb;
            assert(nextBb != fn_->end());

            auto condVal = translateInstrArgument(*i.cond());

            assert(nextBb->get() == onTrue || nextBb->get() == onFalse);
            if (nextBb->get() == onFalse) {
                dest = onTrue;
                cond = {condVal, /* invert= */ false};
            } else {
                dest = onFalse;
                cond = {condVal, /* invert= */ true};
            }
        }

        bb_->addInstr(std::make_unique<lir::JmpInstr>(dest, std::move(cond)));
    }

    void visitRetInstr(const ir::RetInstr& i) override {
        bb_->addInstr(std::make_unique<lir::SafePointInstr>());

        if (i.value().has_value()) {
            bb_->addInstr(std::make_unique<lir::MovInstr>(
                lir::PhysicalRegister::create(lir::PhysicalRegister::Name::Rax),
                translateInstrArgument(**i.value())));
        }

        bb_->addInstr(std::make_unique<lir::RetInstr>());
    }

public:
    Lowerer() = default;

    lir::Module run(const ir::Module& module) {
        visit(module);
        return std::move(res_);
    }
};
}  // namespace

namespace backend {
lir::Module lowerIr(const ir::Module& mod) {
    return Lowerer{}.run(mod);
}
}  // namespace backend
