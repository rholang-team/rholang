#pragma once

#include "compiler/ir/visitor.hpp"

namespace ir {
class PrettyPrinter final : public Visitor<void, true> {
    size_t tmpIdx_ = 0;
    std::unordered_map<const Instr*, size_t> valueNames_;
    std::unordered_map<const BasicBlock*, size_t> bbs_;

    std::ostream& os_;

    void printInstrArg(const Value& v);
    void printInstrArg(const Value* v);

    using Super = Visitor<void, true>;

public:
    using Super::visit;

    PrettyPrinter(std::ostream& os) : os_{os} {}

    void visit(const Module& mod) override;
    void visit(const Function& fn) override;
    void visit(const BasicBlock& bb) override;

    void visitGlobalDecl(const GlobalPtr& global) override;

    void visitInstr(const Instr* i) override;

    void visitIntImm(const IntImm& imm) override;
    void visitBoolImm(const BoolImm& imm) override;
    void visitFnArgRef(const FnArgRef& argRef) override;
    void visitGlobalPtr(const GlobalPtr& globalPtr) override;
    void visitNullPtr(const NullPtr& null) override;

    void visitAllocaInstr(const AllocaInstr& i) override;
    void visitNewInstr(const NewInstr& i) override;
    void visitCallInstr(const CallInstr& i) override;
    void visitNotInstr(const NotInstr& i) override;
    void visitNegInstr(const NegInstr& i) override;
    void visitLoadInstr(const LoadInstr& i) override;
    void visitStoreInstr(const StoreInstr& i) override;
    void visitAddInstr(const AddInstr& i) override;
    void visitSubInstr(const SubInstr& i) override;
    void visitMulInstr(const MulInstr& i) override;
    void visitDivInstr(const DivInstr& i) override;
    void visitCmpInstr(const CmpInstr& i) override;
    void visitGetFieldPtrInstr(const GetFieldPtrInstr& i) override;
    void visitGotoInstr(const GotoInstr& i) override;
    void visitBrInstr(const BrInstr& i) override;
    void visitRetInstr(const RetInstr& i) override;
};
}  // namespace ir
