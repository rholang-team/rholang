#pragma once

#include "compiler/lir/visitor.hpp"

namespace lir {
class PrettyPrinter final : public Visitor<void, true> {
    std::unordered_map<const BasicBlock*, size_t> bbs_;
    std::ostream& os_;

    using Super = Visitor<void, true>;
    using Super::ArgPtr;
    using Super::ArgRef;

public:
    using Super::visit;

    PrettyPrinter(std::ostream& os) : os_{os} {}

    void visit(const Module& mod) override;
    void visit(const Function& fn) override;
    void visit(const BasicBlock& bb) override;
    void visitInstr(const Instr& i) override;

    void visitGlobalDecl(std::string_view name, size_t size) override;

    void visitImmediate(const Immediate& imm) override;

    void visitStackSlot(const StackSlot& slot) override;

    void visitGlobal(const Global& global) override;

    void visitInstr(const Instr* i) override;

    void visitVirtualRegister(const VirtualRegister& i) override;
    void visitPhysicalRegister(const PhysicalRegister& i) override;

    void visitAddressExpression(const AddressExpression& i) override;

    void visitMovInstr(const MovInstr& i) override;
    void visitCallInstr(const CallInstr& i) override;
    void visitPushInstr(const PushInstr& i) override;
    void visitLeaInstr(const LeaInstr& i) override;
    void visitPopInstr(const PopInstr& i) override;
    void visitLoadInstr(const LoadInstr& i) override;
    void visitLoadImmInstr(const LoadImmInstr& i) override;
    void visitStoreInstr(const StoreInstr& i) override;
    void visitCmpInstr(const CmpInstr& i) override;
    void visitAddInstr(const AddInstr& i) override;
    void visitSubInstr(const SubInstr& i) override;
    void visitMulInstr(const MulInstr& i) override;
    void visitDivInstr(const DivInstr& i) override;
    void visitRetInstr(const RetInstr& i) override;
    void visitJmpInstr(const JmpInstr& i) override;
};
}  // namespace lir
