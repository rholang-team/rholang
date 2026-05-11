#pragma once

#include <utility>

#include "compiler/lir/instr.hpp"
#include "compiler/lir/module.hpp"
#include "compiler/lir/register.hpp"

namespace lir {
template <typename RetTy = void, bool Const = false>
struct Visitor {
    template <typename T>
    using ArgPtr = std::conditional_t<Const, const T*, T*>;

    template <typename T>
    using ArgRef = std::conditional_t<Const, const T&, T&>;

    virtual ~Visitor() = default;

    virtual void visit(ArgRef<Module> module) {
        for (auto&& fn : module.functions()) {
            visit(fn);
        }

        for (auto&& [name, global] : module.globals()) {
            visitGlobalDecl(name, global);
        }
    }

    virtual void visitGlobalDecl([[maybe_unused]] std::string_view name,
                                 [[maybe_unused]] WordType w) {}

    virtual void visit(ArgRef<Function> fn) {
        for (auto&& bb : fn) {
            visit(*bb);
        }
    }

    virtual void visit(ArgRef<BasicBlock> bb) {
        for (auto&& i : bb) {
            visitInstr(i.get());
        }
    }

    virtual RetTy visitAddress(ArgPtr<Address> addr) {
        if (ArgPtr<AddressExpression> addrExpr =
                dynamic_cast<ArgPtr<AddressExpression>>(addr)) {
            return visitAddressExpression(*addrExpr);
        } else if (ArgPtr<Register> reg =
                       dynamic_cast<ArgPtr<Register>>(addr)) {
            return visitRegister(reg);
        } else if (ArgPtr<GlobalRef> global =
                       dynamic_cast<ArgPtr<GlobalRef>>(addr)) {
            return visitGlobalRef(*global);
        }

        std::unreachable();
    }

    virtual RetTy visitInstr(ArgRef<Instr> i) {
        return visitInstr(&i);
    }

    virtual RetTy visit(ArgRef<Value> v) {
        return visit(&v);
    }

    virtual RetTy visit(ArgPtr<Value> v) {
        if (ArgPtr<Immediate> imm = dynamic_cast<ArgPtr<Immediate>>(v))
            return visitImmediate(*imm);
        else if (ArgPtr<Address> addr = dynamic_cast<ArgPtr<Address>>(v))
            return visitAddress(addr);
        // else if (ArgPtr<StackSlot> stackSlot =
        //              dynamic_cast<ArgPtr<StackSlot>>(v))
        //     return visitStackSlot(*stackSlot);
        else if (ArgPtr<GlobalRef> global = dynamic_cast<ArgPtr<GlobalRef>>(v))
            return visitGlobalRef(*global);
        else if (ArgPtr<Instr> instr = dynamic_cast<ArgPtr<Instr>>(v))
            return visitInstr(instr);

        std::unreachable();
    }

    virtual RetTy visitRegister(ArgPtr<Register> r) {
        if (ArgPtr<VirtualRegister> vreg =
                dynamic_cast<ArgPtr<VirtualRegister>>(r))
            return visitVirtualRegister(*vreg);
        else if (ArgPtr<PhysicalRegister> preg =
                     dynamic_cast<ArgPtr<PhysicalRegister>>(r))
            return visitPhysicalRegister(*preg);
        else if (ArgPtr<StackPointer> sp =
                     dynamic_cast<ArgPtr<StackPointer>>(r))
            return visitStackPointer(*sp);

        std::unreachable();
    }

    virtual RetTy visitInstr(ArgPtr<Instr> i) {
        if (ArgPtr<MovInstr> movInstr = dynamic_cast<ArgPtr<MovInstr>>(i)) {
            return visitMovInstr(*movInstr);
        } else if (ArgPtr<SafePointInstr> spInstr =
                       dynamic_cast<ArgPtr<SafePointInstr>>(i)) {
            return visitSafePointInstr(*spInstr);
        } else if (ArgPtr<FrameEntryInstr> feInstr =
                       dynamic_cast<ArgPtr<FrameEntryInstr>>(i)) {
            return visitFrameEntryInstr(*feInstr);
        } else if (ArgPtr<PushInstr> pushInstr =
                       dynamic_cast<ArgPtr<PushInstr>>(i)) {
            return visitPushInstr(*pushInstr);
        } else if (ArgPtr<PopInstr> popInstr =
                       dynamic_cast<ArgPtr<PopInstr>>(i)) {
            return visitPopInstr(*popInstr);
        } else if (ArgPtr<LeaInstr> leaInstr =
                       dynamic_cast<ArgPtr<LeaInstr>>(i)) {
            return visitLeaInstr(*leaInstr);
        } else if (ArgPtr<CmpInstr> cmpInstr =
                       dynamic_cast<ArgPtr<CmpInstr>>(i)) {
            return visitCmpInstr(*cmpInstr);
        } else if (ArgPtr<JmpInstr> jmpInstr =
                       dynamic_cast<ArgPtr<JmpInstr>>(i)) {
            return visitJmpInstr(*jmpInstr);
        } else if (ArgPtr<StoreInstr> storeInstr =
                       dynamic_cast<ArgPtr<StoreInstr>>(i)) {
            return visitStoreInstr(*storeInstr);
        } else if (ArgPtr<LoadInstr> loadInstr =
                       dynamic_cast<ArgPtr<LoadInstr>>(i)) {
            return visitLoadInstr(*loadInstr);
        } else if (ArgPtr<LoadImmInstr> loadImmInstr =
                       dynamic_cast<ArgPtr<LoadImmInstr>>(i)) {
            return visitLoadImmInstr(*loadImmInstr);
        } else if (ArgPtr<CallInstr> callInstr =
                       dynamic_cast<ArgPtr<CallInstr>>(i)) {
            return visitCallInstr(*callInstr);
        } else if (ArgPtr<RetInstr> retInstr =
                       dynamic_cast<ArgPtr<RetInstr>>(i)) {
            return visitRetInstr(*retInstr);
        } else if (ArgPtr<AddInstr> addInstr =
                       dynamic_cast<ArgPtr<AddInstr>>(i)) {
            return visitAddInstr(*addInstr);
        } else if (ArgPtr<SubInstr> subInstr =
                       dynamic_cast<ArgPtr<SubInstr>>(i)) {
            return visitSubInstr(*subInstr);
        } else if (ArgPtr<MulInstr> mulInstr =
                       dynamic_cast<ArgPtr<MulInstr>>(i)) {
            return visitMulInstr(*mulInstr);
        } else if (ArgPtr<DivInstr> divInstr =
                       dynamic_cast<ArgPtr<DivInstr>>(i)) {
            return visitDivInstr(*divInstr);
        }

        std::unreachable();
    }

#define MKVISITOR(TY)                                      \
    virtual RetTy visit##TY([[maybe_unused]] ArgRef<TY>) { \
        if constexpr (!std::is_void_v<RetTy>)              \
            return RetTy();                                \
    }

    MKVISITOR(Immediate)
    MKVISITOR(GlobalRef)

    MKVISITOR(VirtualRegister)
    MKVISITOR(PhysicalRegister)
    MKVISITOR(StackPointer)

    MKVISITOR(AddressExpression)

    MKVISITOR(MovInstr)
    MKVISITOR(SafePointInstr)
    MKVISITOR(FrameEntryInstr)
    MKVISITOR(PushInstr)
    MKVISITOR(PopInstr)
    MKVISITOR(LeaInstr)
    MKVISITOR(LoadInstr)
    MKVISITOR(LoadImmInstr)
    MKVISITOR(StoreInstr)
    MKVISITOR(CallInstr)
    MKVISITOR(CmpInstr)
    MKVISITOR(AddInstr)
    MKVISITOR(SubInstr)
    MKVISITOR(MulInstr)
    MKVISITOR(DivInstr)
    MKVISITOR(JmpInstr)
    MKVISITOR(RetInstr)

#undef MKVISITOR
};
}  // namespace lir
