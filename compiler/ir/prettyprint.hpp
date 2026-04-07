#pragma once

#include "compiler/ir/visitor.hpp"

namespace ir {
class PrettyPrinter final : public Visitor<void> {
    size_t tmpIdx_ = 0;
    std::unordered_map<Instr*, size_t> valueNames_;
    std::unordered_map<BasicBlock*, size_t> bbs_;

    std::ostream& os_;

    void printInstrArg(Value& v);
    void printInstrArg(Value* v);

public:
    using Visitor<void>::visit;

    PrettyPrinter(std::ostream& os) : os_{os} {}

    void visit(Module& mod) override;
    void visit(Function& fn) override;
    void visit(BasicBlock& bb) override;

    void visitInstr(Instr* i) override;

    void visitIntImm(IntImm& imm) override;
    void visitBoolImm(BoolImm& imm) override;
    void visitFnArgRef(FnArgRef& argRef) override;
    void visitGlobalPtr(GlobalPtr& globalPtr) override;
    void visitNullPtr(NullPtr& argRef) override;

    void visitAllocaInstr(AllocaInstr& i) override;
    void visitNewInstr(NewInstr& i) override;
    void visitCallInstr(CallInstr& i) override;
    void visitNotInstr(NotInstr& i) override;
    void visitNegInstr(NegInstr& i) override;
    void visitLoadInstr(LoadInstr& i) override;
    void visitStoreInstr(StoreInstr& i) override;
    void visitAddInstr(AddInstr& i) override;
    void visitSubInstr(SubInstr& i) override;
    void visitMulInstr(MulInstr& i) override;
    void visitDivInstr(DivInstr& i) override;
    void visitCmpInstr(CmpInstr& i) override;
    void visitGetFieldPtrInstr(GetFieldPtrInstr& i) override;
    void visitGotoInstr(GotoInstr& i) override;
    void visitBrInstr(BrInstr& i) override;
    void visitRetInstr(RetInstr& i) override;
};
}  // namespace ir
