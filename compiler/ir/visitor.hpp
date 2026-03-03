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
        for (auto& bb : fn.bbs()) {
            visit(*bb);
        }
    }

    virtual void visit(BasicBlock& bb) {
        for (auto& i : bb.instrs()) {
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

    virtual RetTy visitIntImm([[maybe_unused]] IntImm& imm) {}
    virtual RetTy visitBoolImm([[maybe_unused]] BoolImm& imm) {}
    virtual RetTy visitFnArgRef([[maybe_unused]] FnArgRef& argRef) {}

    virtual RetTy visitAllocaInstr([[maybe_unused]] AllocaInstr& i) {}
    virtual RetTy visitNewInstr([[maybe_unused]] NewInstr& i) {}
    virtual RetTy visitCallInstr([[maybe_unused]] CallInstr& i) {}
    virtual RetTy visitNotInstr([[maybe_unused]] NotInstr& i) {}
    virtual RetTy visitNegInstr([[maybe_unused]] NegInstr& i) {}
    virtual RetTy visitLoadInstr([[maybe_unused]] LoadInstr& i) {}
    virtual RetTy visitStoreInstr([[maybe_unused]] StoreInstr& i) {}
    virtual RetTy visitAddInstr([[maybe_unused]] AddInstr& i) {}
    virtual RetTy visitSubInstr([[maybe_unused]] SubInstr& i) {}
    virtual RetTy visitMulInstr([[maybe_unused]] MulInstr& i) {}
    virtual RetTy visitCmpInstr([[maybe_unused]] CmpInstr& i) {}
    virtual RetTy visitGetFieldPtrInstr([[maybe_unused]] GetFieldPtrInstr& i) {}
    virtual RetTy visitGotoInstr([[maybe_unused]] GotoInstr& i) {}
    virtual RetTy visitBrInstr([[maybe_unused]] BrInstr& i) {}
    virtual RetTy visitRetInstr([[maybe_unused]] RetInstr& i) {}
};
}  // namespace ir
