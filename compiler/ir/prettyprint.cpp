#include "compiler/ir/prettyprint.hpp"

#include "utils/match.hpp"

namespace ir {
void PrettyPrinter::visit(const Module& mod) {
    bool first = true;
    for (auto& [_, global] : mod.globals()) {
        if (!first)
            os_ << '\n';
        first = false;

        visitGlobalDecl(*global);
    }

    for (auto& [_, fn] : mod.functions()) {
        if (!first)
            os_ << '\n';
        first = false;
        visit(*fn);
    }
}

void PrettyPrinter::visit(const Function& fn) {
    tmpIdx_ = 0;
    valueNames_.clear();
    bbs_.clear();

    auto& sig = *fn.signature();
    const FunctionType* ty = sig.type();
    os_ << *ty->rettype() << ' ' << sig.name() << '(';

    size_t paramIdx = 0;
    for (Type* param : ty->params()) {
        if (paramIdx != 0)
            os_ << ", ";
        os_ << *param << " a" << paramIdx;
        paramIdx++;
    }
    os_ << "):\n";

    valueNames_.clear();
    bbs_.clear();

    size_t bbIndex = 0;
    for (auto& bb : fn) {
        bbs_.emplace(bb.get(), bbIndex++);
    }

    Super::visit(fn);
}

void PrettyPrinter::visit(const BasicBlock& bb) {
    os_ << "bb" << bbs_.at(&bb) << ":\n";
    Super::visit(bb);
}

void PrettyPrinter::visitGlobalDecl(const GlobalPtr& global) {
    os_ << "global " << *global.valueTy() << ' ' << global.name() << '\n';
}

void PrettyPrinter::visitInstr(const Instr* i) {
    os_ << "  ";
    Super::visitInstr(i);
}

void PrettyPrinter::visitIntImm(const IntImm& imm) {
    os_ << imm.value();
}

void PrettyPrinter::visitBoolImm(const BoolImm& imm) {
    os_ << std::boolalpha << imm.value();
}

void PrettyPrinter::visitFnArgRef(const FnArgRef& argRef) {
    os_ << 'a' << argRef.idx();
}

void PrettyPrinter::visitGlobalPtr(const GlobalPtr& globalPtr) {
    os_ << globalPtr.name();
}

void PrettyPrinter::visitNullPtr(const NullPtr&) {
    os_ << "null";
}

void PrettyPrinter::printInstrArg(const Value& v) {
    printInstrArg(&v);
}

void PrettyPrinter::printInstrArg(const Value* v) {
    if (const IntImm* intImm = dynamic_cast<const IntImm*>(v))
        visitIntImm(*intImm);
    else if (const BoolImm* boolImm = dynamic_cast<const BoolImm*>(v))
        visitBoolImm(*boolImm);
    else if (const FnArgRef* fnArgRef = dynamic_cast<const FnArgRef*>(v))
        visitFnArgRef(*fnArgRef);
    else if (const GlobalPtr* globalPtr = dynamic_cast<const GlobalPtr*>(v))
        visitGlobalPtr(*globalPtr);
    else if (const NullPtr* nullPtr = dynamic_cast<const NullPtr*>(v))
        visitNullPtr(*nullPtr);
    else if (const Instr* instr = dynamic_cast<const Instr*>(v))
        os_ << 'x' << valueNames_.at(instr);
    else
        std::unreachable();
}

void PrettyPrinter::visitAllocaInstr(const AllocaInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = alloca " << *i.itemType() << '\n';
}

void PrettyPrinter::visitNewInstr(const NewInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = new " << *i.itemType() << '\n';
}

void PrettyPrinter::visitCallInstr(const CallInstr& i) {
    if (!utils::isa<VoidType>(i.callee()->type()->rettype())) {
        valueNames_.emplace(&i, tmpIdx_++);
        printInstrArg(i);
        os_ << " = ";
    }

    os_ << "call " << i.callee()->name();
    for (auto& arg : i.args()) {
        os_ << ' ';
        printInstrArg(arg.get());
    }
    os_ << '\n';
}

void PrettyPrinter::visitNotInstr(const NotInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = not ";
    printInstrArg(i.target().get());
    os_ << '\n';
}

void PrettyPrinter::visitNegInstr(const NegInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = neg ";
    printInstrArg(i.target().get());
    os_ << '\n';
}

void PrettyPrinter::visitLoadInstr(const LoadInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = load " << *i.type() << ' ';
    printInstrArg(i.src().get());
    os_ << '\n';
}

void PrettyPrinter::visitStoreInstr(const StoreInstr& i) {
    os_ << "store " << *i.storedValueType() << ' ';
    printInstrArg(i.dest().get());
    os_ << ' ';
    printInstrArg(i.src().get());
    os_ << '\n';
}

void PrettyPrinter::visitAddInstr(const AddInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = add ";
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitSubInstr(const SubInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = sub ";
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitMulInstr(const MulInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = mul ";
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitDivInstr(const DivInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = div ";
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitCmpInstr(const CmpInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = cmp ";

    switch (i.cond()) {
        case CmpInstr::Cond::Eq:
            os_ << "eq";
            break;
        case CmpInstr::Cond::Ne:
            os_ << "ne";
            break;
        case CmpInstr::Cond::Lt:
            os_ << "lt";
            break;
        case CmpInstr::Cond::Gt:
            os_ << "gt";
            break;
        case CmpInstr::Cond::Le:
            os_ << "le";
            break;
        case CmpInstr::Cond::Ge:
            os_ << "ge";
            break;
    }

    os_ << ' ';
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitGetFieldPtrInstr(const GetFieldPtrInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = gfp " << *i.structType() << ' ';
    printInstrArg(i.target().get());
    os_ << ' ' << i.fieldIdx() << '\n';
}

void PrettyPrinter::visitGotoInstr(const GotoInstr& i) {
    os_ << "goto bb" << bbs_.at(i.dest()) << '\n';
}

void PrettyPrinter::visitBrInstr(const BrInstr& i) {
    os_ << "br ";
    printInstrArg(i.cond().get());
    os_ << " bb" << bbs_.at(i.onTrue()) << " bb" << bbs_.at(i.onFalse())
        << '\n';
}

void PrettyPrinter::visitRetInstr(const RetInstr& i) {
    os_ << "ret";

    if (i.value().has_value()) {
        os_ << ' ';
        printInstrArg(i.value()->get());
    }
    os_ << '\n';
}
}  // namespace ir
