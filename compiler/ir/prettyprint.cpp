#include "compiler/ir/prettyprint.hpp"

#include "utils/match.hpp"

namespace ir {
void PrettyPrinter::visit(Module& mod) {
    bool first = true;
    for (auto& [_, global] : mod.globals()) {
        if (!first)
            os_ << '\n';
        first = false;

        os_ << "global " << *global->valueTy() << ' ' << global->name() << '\n';
    }

    for (auto& [_, fn] : mod.functions()) {
        if (!first)
            os_ << '\n';
        first = false;
        visit(*fn);
    }
}

void PrettyPrinter::visit(Function& fn) {
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

    Visitor<void>::visit(fn);
}

void PrettyPrinter::visit(BasicBlock& bb) {
    os_ << "bb" << bbs_.at(&bb) << ":\n";
    Visitor<void>::visit(bb);
}

void PrettyPrinter::visitInstr(Instr* i) {
    os_ << "  ";
    Visitor<void>::visitInstr(i);
}

void PrettyPrinter::visitIntImm(IntImm& imm) {
    os_ << imm.value();
}

void PrettyPrinter::visitBoolImm(BoolImm& imm) {
    os_ << std::boolalpha << imm.value();
}

void PrettyPrinter::visitFnArgRef(FnArgRef& argRef) {
    os_ << 'a' << argRef.idx();
}

void PrettyPrinter::visitGlobalPtr(GlobalPtr& globalPtr) {
    os_ << globalPtr.name();
}

void PrettyPrinter::visitNullPtr(NullPtr&) {
    os_ << "null";
}

void PrettyPrinter::printInstrArg(Value& v) {
    printInstrArg(&v);
}

void PrettyPrinter::printInstrArg(Value* v) {
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
        os_ << 'x' << valueNames_.at(instr);
    else
        std::unreachable();
}

void PrettyPrinter::visitAllocaInstr(AllocaInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = alloca " << *i.itemType() << '\n';
}

void PrettyPrinter::visitNewInstr(NewInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = new " << *i.itemType() << '\n';
}

void PrettyPrinter::visitCallInstr(CallInstr& i) {
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

void PrettyPrinter::visitNotInstr(NotInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = not ";
    printInstrArg(i.target().get());
    os_ << '\n';
}

void PrettyPrinter::visitNegInstr(NegInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = neg ";
    printInstrArg(i.target().get());
    os_ << '\n';
}

void PrettyPrinter::visitLoadInstr(LoadInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = load " << *i.type() << ' ';
    printInstrArg(i.src().get());
    os_ << '\n';
}

void PrettyPrinter::visitStoreInstr(StoreInstr& i) {
    os_ << "store " << *i.storedValueType() << ' ';
    printInstrArg(i.dest().get());
    os_ << ' ';
    printInstrArg(i.src().get());
    os_ << '\n';
}

void PrettyPrinter::visitAddInstr(AddInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = add ";
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitSubInstr(SubInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = sub ";
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitMulInstr(MulInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = mul ";
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitDivInstr(DivInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = div ";
    printInstrArg(i.lhs().get());
    os_ << ' ';
    printInstrArg(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitCmpInstr(CmpInstr& i) {
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

void PrettyPrinter::visitGetFieldPtrInstr(GetFieldPtrInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printInstrArg(i);
    os_ << " = gfp " << *i.structType() << ' ';
    printInstrArg(i.target().get());
    os_ << ' ' << i.fieldIdx() << '\n';
}

void PrettyPrinter::visitGotoInstr(GotoInstr& i) {
    os_ << "goto bb" << bbs_.at(i.dest()) << '\n';
}

void PrettyPrinter::visitBrInstr(BrInstr& i) {
    os_ << "br ";
    printInstrArg(i.cond().get());
    os_ << " bb" << bbs_.at(i.onTrue()) << " bb" << bbs_.at(i.onFalse())
        << '\n';
}

void PrettyPrinter::visitRetInstr(RetInstr& i) {
    os_ << "ret";

    if (i.value().has_value()) {
        os_ << ' ';
        printInstrArg(i.value()->get());
    }
    os_ << '\n';
}
}  // namespace ir
