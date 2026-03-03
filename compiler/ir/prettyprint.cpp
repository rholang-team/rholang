#include "compiler/ir/prettyprint.hpp"

#include "utils/match.hpp"

namespace ir {
void PrettyPrinter::visit(Module& mod) {
    bool first = true;
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
    for (auto& bb : fn.bbs()) {
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

void PrettyPrinter::visitNullPtr(NullPtr&) {
    os_ << "null";
}

void PrettyPrinter::printTmp(Value& v) {
    printTmp(&v);
}

void PrettyPrinter::printTmp(Value* v) {
    if (IntImm* intImm = dynamic_cast<IntImm*>(v))
        visitIntImm(*intImm);
    else if (BoolImm* boolImm = dynamic_cast<BoolImm*>(v))
        visitBoolImm(*boolImm);
    else if (FnArgRef* fnArgRef = dynamic_cast<FnArgRef*>(v))
        visitFnArgRef(*fnArgRef);
    else if (NullPtr* nullPtr = dynamic_cast<NullPtr*>(v))
        visitNullPtr(*nullPtr);
    else if (Instr* instr = dynamic_cast<Instr*>(v))
        os_ << 'x' << valueNames_.at(instr);
    else
        std::unreachable();
}

void PrettyPrinter::visitAllocaInstr(AllocaInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = alloca " << *i.itemType() << '\n';
}

void PrettyPrinter::visitNewInstr(NewInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = new " << *i.itemType() << '\n';
}

void PrettyPrinter::visitCallInstr(CallInstr& i) {
    if (!utils::isa<VoidType>(i.callee()->type()->rettype())) {
        valueNames_.emplace(&i, tmpIdx_++);
        printTmp(i);
        os_ << " = ";
    }

    os_ << "call " << i.callee()->name();
    for (auto& arg : i.args()) {
        os_ << ' ';
        printTmp(arg.get());
    }
    os_ << '\n';
}

void PrettyPrinter::visitNotInstr(NotInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = not ";
    printTmp(i.target().get());
    os_ << '\n';
}

void PrettyPrinter::visitNegInstr(NegInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = neg ";
    printTmp(i.target().get());
    os_ << '\n';
}

void PrettyPrinter::visitLoadInstr(LoadInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = load " << *i.type() << ' ';
    printTmp(i.src().get());
    os_ << '\n';
}

void PrettyPrinter::visitStoreInstr(StoreInstr& i) {
    os_ << "store " << *i.storedValueType() << ' ';
    printTmp(i.dest().get());
    os_ << ' ';
    printTmp(i.src().get());
    os_ << '\n';
}

void PrettyPrinter::visitAddInstr(AddInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = add ";
    printTmp(i.lhs().get());
    os_ << ' ';
    printTmp(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitSubInstr(SubInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = sub ";
    printTmp(i.lhs().get());
    os_ << ' ';
    printTmp(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitMulInstr(MulInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = mul ";
    printTmp(i.lhs().get());
    os_ << ' ';
    printTmp(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitCmpInstr(CmpInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = cmp ";

    switch (i.cond()) {
        case CmpInstr::Cond::Eq:
            os_ << "==";
            break;
        case CmpInstr::Cond::Ne:
            os_ << "!=";
            break;
        case CmpInstr::Cond::Lt:
            os_ << "<";
            break;
        case CmpInstr::Cond::Gt:
            os_ << ">";
            break;
        case CmpInstr::Cond::Le:
            os_ << "<=";
            break;
        case CmpInstr::Cond::Ge:
            os_ << ">=";
            break;
    }

    os_ << ' ';
    printTmp(i.lhs().get());
    os_ << ' ';
    printTmp(i.rhs().get());
    os_ << '\n';
}

void PrettyPrinter::visitGetFieldPtrInstr(GetFieldPtrInstr& i) {
    valueNames_.emplace(&i, tmpIdx_++);
    printTmp(i);
    os_ << " = gfp " << *i.structType() << ' ';
    printTmp(i.target().get());
    os_ << ' ' << i.fieldIdx() << '\n';
}

void PrettyPrinter::visitGotoInstr(GotoInstr& i) {
    os_ << "goto bb" << bbs_.at(i.dest()) << '\n';
}

void PrettyPrinter::visitBrInstr(BrInstr& i) {
    os_ << "br ";
    printTmp(i.cond().get());
    os_ << " bb" << bbs_.at(i.onTrue()) << " bb" << bbs_.at(i.onFalse())
        << '\n';
}

void PrettyPrinter::visitRetInstr(RetInstr& i) {
    os_ << "ret";

    if (i.value().has_value()) {
        os_ << ' ';
        printTmp(i.value()->get());
    }
    os_ << '\n';
}
}  // namespace ir
