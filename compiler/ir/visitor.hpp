#pragma once

#include <utility>

#include "compiler/ir/instr.hpp"
#include "compiler/ir/module.hpp"

namespace ir {
template <typename RetTy = void>
struct Visitor {
    virtual ~Visitor() = default;

    virtual void visit(Module& module) {
        for (auto& [_, fn] : module.functions()) {
            visit(*fn);
        }
    }

    virtual void visit(Function& fn) {
        for (auto& bb : fn) {
            visit(*bb);
        }
    }

    virtual void visit(BasicBlock& bb) {
        for (auto& i : bb) {
            visit(i.get());
        }
    }

    virtual RetTy visitInstr(Instr& i) {
        return visit(&i);
    }

    virtual RetTy visit(Value& v) {
        return visit(&v);
    }

    virtual RetTy visit(Value* v) {
        if (IntImm* intImm = dynamic_cast<IntImm*>(v))
            visitIntImm(*intImm);
        else if (BoolImm* boolImm = dynamic_cast<BoolImm*>(v))
            visitBoolImm(*boolImm);
        else if (FnArgRef* fnArgRef = dynamic_cast<FnArgRef*>(v))
            visitFnArgRef(*fnArgRef);
        else if (GlobalPtr* globalPtr = dynamic_cast<GlobalPtr*>(v))
            visitGlobalPtr(*globalPtr);
        else if (NullPtr* nullPtr = dynamic_cast<NullPtr*>(v))
            visitNullPtr(*nullPtr);
        else if (Instr* instr = dynamic_cast<Instr*>(v))
            visitInstr(instr);
        else
            std::unreachable();
    }

    virtual RetTy visitInstr(Instr* i) {
        if (AllocaInstr* allocaInstr = dynamic_cast<AllocaInstr*>(i))
            return visitAllocaInstr(*allocaInstr);
        else if (NewInstr* allocaInstr = dynamic_cast<NewInstr*>(i))
            return visitNewInstr(*allocaInstr);
        else if (CallInstr* callInstr = dynamic_cast<CallInstr*>(i))
            return visitCallInstr(*callInstr);
        else if (NotInstr* notInstr = dynamic_cast<NotInstr*>(i))
            return visitNotInstr(*notInstr);
        else if (NegInstr* negInstr = dynamic_cast<NegInstr*>(i))
            return visitNegInstr(*negInstr);
        else if (LoadInstr* loadInstr = dynamic_cast<LoadInstr*>(i))
            return visitLoadInstr(*loadInstr);
        else if (StoreInstr* storeInstr = dynamic_cast<StoreInstr*>(i))
            return visitStoreInstr(*storeInstr);
        else if (AddInstr* addInstr = dynamic_cast<AddInstr*>(i))
            return visitAddInstr(*addInstr);
        else if (SubInstr* subInstr = dynamic_cast<SubInstr*>(i))
            return visitSubInstr(*subInstr);
        else if (MulInstr* mulInstr = dynamic_cast<MulInstr*>(i))
            return visitMulInstr(*mulInstr);
        else if (DivInstr* divInstr = dynamic_cast<DivInstr*>(i))
            return visitDivInstr(*divInstr);
        else if (CmpInstr* cmpInstr = dynamic_cast<CmpInstr*>(i))
            return visitCmpInstr(*cmpInstr);
        else if (GetFieldPtrInstr* getFieldPtrInstr =
                     dynamic_cast<GetFieldPtrInstr*>(i))
            return visitGetFieldPtrInstr(*getFieldPtrInstr);
        else if (GotoInstr* gotoInstr = dynamic_cast<GotoInstr*>(i))
            return visitGotoInstr(*gotoInstr);
        else if (BrInstr* brInstr = dynamic_cast<BrInstr*>(i))
            return visitBrInstr(*brInstr);
        else if (RetInstr* retInstr = dynamic_cast<RetInstr*>(i))
            return visitRetInstr(*retInstr);

        std::unreachable();
    }

#define MKVISITOR(TY)                                   \
    virtual RetTy visit##TY([[maybe_unused]] TY& imm) { \
        if constexpr (!std::is_void_v<RetTy>)           \
            return RetTy();                             \
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
