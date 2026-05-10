#include "compiler/lir/prettyprint.hpp"

#include "compiler/lir/register.hpp"
#include "compiler/lir/value.hpp"

namespace lir {
void PrettyPrinter::visit(const Module& mod) {
    bool first = true;
    for (auto&& [name, global] : mod.globals()) {
        first = false;
        visitGlobalDecl(name, global);
    }

    for (auto&& fn : mod.functions()) {
        if (!first)
            os_ << '\n';
        first = false;
        visit(fn);
    }
}

void PrettyPrinter::visitGlobalDecl(std::string_view name, WordType w) {
    os_ << "global " << wordTypeToString(w) << ' ' << name << '\n';
}

void PrettyPrinter::visit(const Function& fn) {
    os_ << fn.label() << ":\n";

    bbs_.clear();

    size_t idx = 0;
    for (auto&& bb : fn.bbs()) {
        bbs_.emplace(bb.get(), idx++);
    }

    for (auto&& bb : fn.bbs()) {
        visit(*bb);
    }
}

void PrettyPrinter::visit(const BasicBlock& bb) {
    os_ << "bb" << bbs_.at(&bb) << ":\n";
    Super::visit(bb);
}

void PrettyPrinter::visitInstr(const Instr& i) {
    visitInstr(&i);
}

void PrettyPrinter::visitInstr(const Instr* i) {
    os_ << "  ";
    Super::visitInstr(i);
    os_ << '\n';
}

void PrettyPrinter::visitVirtualRegister(const VirtualRegister& i) {
    os_ << 'v' << i.id();
}

void PrettyPrinter::visitPhysicalRegister(const PhysicalRegister& i) {
    os_ << i.toString(WordType::Qword);
}

void PrettyPrinter::visitStackPointer(const StackPointer&) {
    os_ << "sp";
}

void PrettyPrinter::visitImmediate(const Immediate& imm) {
    os_ << imm.value();
}
// void PrettyPrinter::visitStackSlot(const StackSlot& slot) {
//     os_ << 's' << slot.slot();
// }

void PrettyPrinter::visitGlobalRef(const GlobalRef& global) {
    os_ << '[' << global.name() << ']';
}

void PrettyPrinter::visitAddressExpression(const AddressExpression& addr) {
    os_ << "[";
    visitRegister(addr.base.get());
    if (addr.displacement() < 0) {
        os_ << " - " << -addr.displacement();
    } else if (addr.displacement() > 0) {
        os_ << " + " << addr.displacement();
    }
    os_ << ']';
}

void PrettyPrinter::visitMovInstr(const MovInstr& i) {
    visitRegister(i.dest.get());
    os_ << " = ";
    visit(i.src.get());
}

void PrettyPrinter::visitCallInstr(const CallInstr& i) {
    os_ << "call " << i.callee();

    // for (auto&& arg : i.args) {
    //     os_ << ' ';
    //     visit(arg.get());
    // }
}

void PrettyPrinter::visitPushInstr(const PushInstr& i) {
    os_ << "push ";
    visitRegister(i.reg.get());
}

void PrettyPrinter::visitLeaInstr(const LeaInstr& i) {
    visitRegister(i.dest.get());
    os_ << " = lea ";
    visitAddress(i.addr.get());
}

void PrettyPrinter::visitPopInstr(const PopInstr& i) {
    visitRegister(i.reg.get());
    os_ << " = pop";
}

void PrettyPrinter::visitLoadInstr(const LoadInstr& i) {
    visitRegister(i.dest.get());
    os_ << " = load " << wordTypeToString(i.itemSize()) << ' ';
    visitAddress(i.src.get());
}

void PrettyPrinter::visitLoadImmInstr(const LoadImmInstr& i) {
    visitRegister(i.dest.get());
    os_ << " = " << i.imm();
}

void PrettyPrinter::visitStoreInstr(const StoreInstr& i) {
    os_ << "store " << wordTypeToString(i.itemSize()) << ' ';
    visitAddress(i.dest.get());
    os_ << ' ';
    visit(i.src.get());
}

void PrettyPrinter::visitCmpInstr(const CmpInstr& i) {
    visitRegister(i.dest.get());
    os_ << " = cmp " << wordTypeToString(i.itemSize) << ' ';
    switch (i.cond) {
        case CmpInstr::Cond::Eq:
            os_ << "eq ";
            break;
        case CmpInstr::Cond::Ne:
            os_ << "ne ";
            break;
        case CmpInstr::Cond::Lt:
            os_ << "lt ";
            break;
        case CmpInstr::Cond::Le:
            os_ << "le ";
            break;
        case CmpInstr::Cond::Gt:
            os_ << "gt ";
            break;
        case CmpInstr::Cond::Ge:
            os_ << "ge ";
            break;
    }
    visit(i.lhs.get());
    os_ << ' ';
    visit(i.rhs.get());
}

void PrettyPrinter::visitAddInstr(const AddInstr& i) {
    os_ << "add ";
    visit(i.lhsDest.get());
    os_ << ' ';
    visit(i.rhs.get());
}
void PrettyPrinter::visitSubInstr(const SubInstr& i) {
    os_ << "sub ";
    visit(i.lhsDest.get());
    os_ << ' ';
    visit(i.rhs.get());
}
void PrettyPrinter::visitMulInstr(const MulInstr& i) {
    os_ << "mul ";
    visit(i.lhsDest.get());
    os_ << ' ';
    visit(i.rhs.get());
}
void PrettyPrinter::visitDivInstr(const DivInstr& i) {
    os_ << "div ";
    visit(i.rhs.get());
}

void PrettyPrinter::visitRetInstr(const RetInstr&) {
    os_ << "ret";
}

void PrettyPrinter::visitJmpInstr(const JmpInstr& i) {
    os_ << "jmp ";

    if (i.conditional()) {
        if (i.invertedCond()) {
            os_ << '!';
        }
        visit(i.cond->first.get());
        os_ << ' ';
    }

    os_ << "bb" << bbs_.at(i.dest());
}
}  // namespace lir
