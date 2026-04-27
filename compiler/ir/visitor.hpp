#pragma once

#include <utility>

#include "compiler/ir/instr.hpp"
#include "compiler/ir/module.hpp"

namespace ir {
template <typename RetTy = void, bool Const = false>
struct Visitor {
    template <typename T>
    using ArgPtr = std::conditional_t<Const, const T*, T*>;

    template <typename T>
    using ArgRef = std::conditional_t<Const, const T&, T&>;

    virtual ~Visitor() = default;

    virtual void visit(ArgRef<Module> module) {
        for (auto&& [_, fn] : module.functions()) {
            visit(*fn);
        }

        for (auto&& [_, global] : module.globals()) {
            visitGlobalDecl(*global);
        }
    }

    virtual void visitGlobalDecl([[maybe_unused]] ArgRef<GlobalPtr> global) {}

    virtual void visit(ArgRef<Function> fn) {
        for (auto&& bb : fn) {
            visit(*bb);
        }
    }

    virtual void visit(ArgRef<BasicBlock> bb) {
        for (auto&& i : bb) {
            visit(i.get());
        }
    }

    virtual RetTy visitInstr(ArgRef<Instr> i) {
        return visit(&i);
    }

    virtual RetTy visit(ArgRef<Value> v) {
        return visit(&v);
    }


    virtual RetTy visit(ArgPtr<Value> v) {
        if (ArgPtr<IntImm> intImm = dynamic_cast<ArgPtr<IntImm>>(v))
            visitIntImm(*intImm);
        else if (ArgPtr<BoolImm> boolImm = dynamic_cast<ArgPtr<BoolImm>>(v))
            visitBoolImm(*boolImm);
        else if (ArgPtr<FnArgRef> fnArgRef = dynamic_cast<ArgPtr<FnArgRef>>(v))
            visitFnArgRef(*fnArgRef);
        else if (ArgPtr<GlobalPtr> globalPtr =
                     dynamic_cast<ArgPtr<GlobalPtr>>(v))
            visitGlobalPtr(*globalPtr);
        else if (ArgPtr<NullPtr> nullPtr = dynamic_cast<ArgPtr<NullPtr>>(v))
            visitNullPtr(*nullPtr);
        else if (ArgPtr<Instr> instr = dynamic_cast<ArgPtr<Instr>>(v))
            visitInstr(instr);
        else
            std::unreachable();
    }

    virtual RetTy visitInstr(ArgPtr<Instr> i) {
        if (ArgPtr<AllocaInstr> allocaInstr =
                dynamic_cast<ArgPtr<AllocaInstr>>(i))
            return visitAllocaInstr(*allocaInstr);
        else if (ArgPtr<NewInstr> allocaInstr =
                     dynamic_cast<ArgPtr<NewInstr>>(i))
            return visitNewInstr(*allocaInstr);
        else if (ArgPtr<CallInstr> callInstr =
                     dynamic_cast<ArgPtr<CallInstr>>(i))
            return visitCallInstr(*callInstr);
        else if (ArgPtr<NotInstr> notInstr = dynamic_cast<ArgPtr<NotInstr>>(i))
            return visitNotInstr(*notInstr);
        else if (ArgPtr<NegInstr> negInstr = dynamic_cast<ArgPtr<NegInstr>>(i))
            return visitNegInstr(*negInstr);
        else if (ArgPtr<LoadInstr> loadInstr =
                     dynamic_cast<ArgPtr<LoadInstr>>(i))
            return visitLoadInstr(*loadInstr);
        else if (ArgPtr<StoreInstr> storeInstr =
                     dynamic_cast<ArgPtr<StoreInstr>>(i))
            return visitStoreInstr(*storeInstr);
        else if (ArgPtr<AddInstr> addInstr = dynamic_cast<ArgPtr<AddInstr>>(i))
            return visitAddInstr(*addInstr);
        else if (ArgPtr<SubInstr> subInstr = dynamic_cast<ArgPtr<SubInstr>>(i))
            return visitSubInstr(*subInstr);
        else if (ArgPtr<MulInstr> mulInstr = dynamic_cast<ArgPtr<MulInstr>>(i))
            return visitMulInstr(*mulInstr);
        else if (ArgPtr<DivInstr> divInstr = dynamic_cast<ArgPtr<DivInstr>>(i))
            return visitDivInstr(*divInstr);
        else if (ArgPtr<CmpInstr> cmpInstr = dynamic_cast<ArgPtr<CmpInstr>>(i))
            return visitCmpInstr(*cmpInstr);
        else if (ArgPtr<GetFieldPtrInstr> getFieldPtrInstr =
                     dynamic_cast<ArgPtr<GetFieldPtrInstr>>(i))
            return visitGetFieldPtrInstr(*getFieldPtrInstr);
        else if (ArgPtr<GotoInstr> gotoInstr =
                     dynamic_cast<ArgPtr<GotoInstr>>(i))
            return visitGotoInstr(*gotoInstr);
        else if (ArgPtr<BrInstr> brInstr = dynamic_cast<ArgPtr<BrInstr>>(i))
            return visitBrInstr(*brInstr);
        else if (ArgPtr<RetInstr> retInstr = dynamic_cast<ArgPtr<RetInstr>>(i))
            return visitRetInstr(*retInstr);

        std::unreachable();
    }

#define MKVISITOR(TY)                                      \
    virtual RetTy visit##TY([[maybe_unused]] ArgRef<TY>) { \
        if constexpr (!std::is_void_v<RetTy>)              \
            return RetTy();                                \
    }

    MKVISITOR(IntImm)
    MKVISITOR(BoolImm)
    MKVISITOR(FnArgRef)
    MKVISITOR(GlobalPtr)
    MKVISITOR(NullPtr)

    MKVISITOR(AllocaInstr)
    MKVISITOR(NewInstr)
    MKVISITOR(CallInstr)
    MKVISITOR(NotInstr)
    MKVISITOR(NegInstr)
    MKVISITOR(LoadInstr)
    MKVISITOR(StoreInstr)
    MKVISITOR(AddInstr)
    MKVISITOR(SubInstr)
    MKVISITOR(MulInstr)
    MKVISITOR(DivInstr)
    MKVISITOR(CmpInstr)
    MKVISITOR(GetFieldPtrInstr)
    MKVISITOR(GotoInstr)
    MKVISITOR(BrInstr)
    MKVISITOR(RetInstr)

#undef MKVISITOR
};  // namespace ir
}  // namespace ir
