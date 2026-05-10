#include "compiler/backend/codegen.hpp"

#include <cassert>

#include "compiler/lir/bb.hpp"
#include "compiler/lir/function.hpp"
#include "compiler/lir/instr.hpp"
#include "compiler/lir/value.hpp"
#include "compiler/lir/visitor.hpp"
#include "utils/match.hpp"

namespace backend {
namespace {
class Generator : private lir::Visitor<void, true> {
public:
    using Super = lir::Visitor<void, true>;

private:
    using Super::visit;

    std::ostream& os_;
    lir::WordType wordType_;

    size_t virtualsCount_;

    size_t physRegIdx_ = 0;
    std::unordered_map<lir::VirtualRegister::Id, lir::PhysicalRegister::Name>
        allocatedRegisters_;

    void storeIfVirtualRegister(const lir::Register* r) {
        const lir::VirtualRegister* vreg =
            dynamic_cast<const lir::VirtualRegister*>(r);

        if (!vreg) {
            return;
        }

        auto name = allocatedRegisters_[vreg->id()];

        size_t offset = lir::wordTypeToSize(lir::WordType::Qword) *
                        (virtualsCount_ - vreg->id());
        os_ << "mov " << lir::wordTypeToString(wordType_) << " [rsp + "
            << offset << "], "
            << lir::PhysicalRegister::nameToString(name, wordType_) << '\n';
    }

    lir::PhysicalRegister::Name allocatePhysicalReg() {
        return lir::PhysicalRegister::names[physRegIdx_++];
    }

    void allocatePhysicalRegIfVirtual(const lir::Register* r) {
        const lir::VirtualRegister* vreg =
            dynamic_cast<const lir::VirtualRegister*>(r);

        if (!vreg) {
            return;
        }

        allocatedRegisters_[vreg->id()] = allocatePhysicalReg();
    }

    void loadVirtualRegs(std::initializer_list<const lir::Register*> r) {
        for (const lir::Register* r : r) {
            const lir::VirtualRegister* vreg =
                dynamic_cast<const lir::VirtualRegister*>(r);

            if (!vreg) {
                continue;
            }

            auto name = allocatePhysicalReg();
            allocatedRegisters_[vreg->id()] = name;

            size_t offset = lir::wordTypeToSize(lir::WordType::Qword) *
                            (virtualsCount_ - vreg->id());
            os_ << "mov "
                << lir::PhysicalRegister::nameToString(name, wordType_) << ", "
                << lir::wordTypeToString(wordType_) << " [rsp + " << offset
                << "]\n";
        }
    }

    void visitGlobalDecl(std::string_view name, lir::WordType w) override {
        os_ << name << ": ";
        switch (w) {
            case lir::WordType::Byte:
                os_ << "db";
                break;
            case lir::WordType::Word:
                os_ << "dw";
                break;
            case lir::WordType::Dword:
                os_ << "dd";
                break;
            case lir::WordType::Qword:
                os_ << "dq";
                break;
        }

        os_ << " 0\n";
    }

    void visit(const lir::Function& fn) override {
        virtualsCount_ = fn.frameMap().size();

        os_ << fn.label() << ":\n";
        Super::visit(fn);
    }

    void visit(const lir::BasicBlock& bb) override {
        os_ << ".bb" << bb.idx() << ":\n";
        Super::visit(bb);
    }

    void visitInstr(const lir::Instr& i) override {
        visitInstr(&i);
    }

    void visitInstr(const lir::Instr* i) override {
        allocatedRegisters_.clear();
        physRegIdx_ = 0;
        Super::visitInstr(i);
    }

    void visitImmediate(const lir::Immediate& imm) override {
        os_ << imm.value();
    }

    void visitGlobalRef(const lir::GlobalRef& global) override {
        os_ << "[rel " << global.name() << ']';
    }

    void visitVirtualRegister(const lir::VirtualRegister& r) override {
        os_ << lir::PhysicalRegister::nameToString(
            allocatedRegisters_.at(r.id()),
            wordType_);
    }

    void visitPhysicalRegister(const lir::PhysicalRegister& r) override {
        os_ << lir::PhysicalRegister::nameToString(r.name(), wordType_);
    }

    void visitStackPointer(const lir::StackPointer&) override {
        os_ << "rsp";
    }

    void visitAddressExpression(const lir::AddressExpression& addr) override {
        os_ << '[';
        wordType_ = lir::WordType::Qword;
        visitRegister(addr.base.get());

        if (addr.displacement() < 0) {
            os_ << " - " << -addr.displacement();
        } else if (addr.displacement() > 0) {
            os_ << " + " << addr.displacement();
        }
        os_ << ']';
    }

    void visitMovInstr(const lir::MovInstr& i) override {
        wordType_ = lir::WordType::Qword;

        const lir::VirtualRegister* vreg =
            dynamic_cast<const lir::VirtualRegister*>(i.dest.get());

        loadVirtualRegs({i.src.get()});
        if (!vreg) {
            os_ << "mov ";
            visitRegister(i.dest.get());
            os_ << ", ";
            visitRegister(i.src.get());
            os_ << '\n';
            return;
        }

        lir::PhysicalRegister::Name name;

        const lir::VirtualRegister* vregSrc =
            dynamic_cast<const lir::VirtualRegister*>(i.src.get());

        if (vregSrc) {
            name = allocatedRegisters_[vregSrc->id()];
        } else {
            name = dynamic_cast<const lir::PhysicalRegister&>(*i.src).name();
        }

        size_t offset = lir::wordTypeToSize(lir::WordType::Qword) *
                        (virtualsCount_ - vreg->id());
        os_ << "mov " << lir::wordTypeToString(wordType_) << " [rsp + "
            << offset << "], "
            << lir::PhysicalRegister::nameToString(name, wordType_) << '\n';
    }

    void visitCallInstr(const lir::CallInstr& i) override {
        os_ << "call " << i.callee() << '\n';
    }

    void visitPushInstr(const lir::PushInstr& i) override {
        loadVirtualRegs({i.reg.get()});

        os_ << "push ";
        wordType_ = lir::WordType::Qword;
        visitRegister(i.reg.get());
        os_ << '\n';
    }

    void visitPopInstr(const lir::PopInstr& i) override {
        allocatePhysicalRegIfVirtual(i.reg.get());

        os_ << "pop ";
        wordType_ = lir::WordType::Qword;
        visitRegister(i.reg.get());
        os_ << '\n';

        storeIfVirtualRegister(i.reg.get());
    }

    void visitLeaInstr(const lir::LeaInstr& i) override {
        wordType_ = lir::WordType::Qword;

        if (const lir::AddressExpression* addr =
                dynamic_cast<const lir::AddressExpression*>(i.addr.get())) {
            loadVirtualRegs({addr->base.get()});
        } else if (const lir::Register* reg =
                       dynamic_cast<const lir::Register*>(i.addr.get())) {
            loadVirtualRegs({reg});
        }

        allocatePhysicalRegIfVirtual(i.dest.get());

        os_ << "lea ";

        visitRegister(i.dest.get());

        os_ << ", ";

        bool shouldWrap = utils::isa<lir::Register>(i.addr.get());
        if (shouldWrap) {
            os_ << '[';
        }
        visitAddress(i.addr.get());
        if (shouldWrap) {
            os_ << ']';
        }
        os_ << '\n';

        storeIfVirtualRegister(i.dest.get());
    }

    void visitLoadInstr(const lir::LoadInstr& i) override {
        wordType_ = lir::WordType::Qword;
        if (const lir::AddressExpression* addr =
                dynamic_cast<const lir::AddressExpression*>(i.src.get())) {
            loadVirtualRegs({addr->base.get()});
        } else if (const lir::Register* reg =
                       dynamic_cast<const lir::Register*>(i.src.get())) {
            loadVirtualRegs({reg});
        }

        allocatePhysicalRegIfVirtual(i.dest.get());

        os_ << "mov ";

        wordType_ = i.itemSize();
        visitRegister(i.dest.get());

        os_ << ", ";

        wordType_ = lir::WordType::Qword;
        bool shouldWrap = utils::isa<lir::Register>(i.src.get());
        if (shouldWrap) {
            os_ << '[';
        }
        visitAddress(i.src.get());
        if (shouldWrap) {
            os_ << ']';
        }
        os_ << '\n';

        storeIfVirtualRegister(i.dest.get());
    }

    void visitLoadImmInstr(const lir::LoadImmInstr& i) override {
        wordType_ = lir::WordType::Dword;

        allocatePhysicalRegIfVirtual(i.dest.get());

        os_ << "mov ";
        visitRegister(i.dest.get());
        os_ << ", " << i.imm() << '\n';

        storeIfVirtualRegister(i.dest.get());
    }

    void visitStoreInstr(const lir::StoreInstr& i) override {
        wordType_ = lir::WordType::Qword;
        if (const lir::AddressExpression* addr =
                dynamic_cast<const lir::AddressExpression*>(i.src.get())) {
            loadVirtualRegs({addr->base.get()});
        } else if (const lir::Register* reg =
                       dynamic_cast<const lir::Register*>(i.src.get())) {
            loadVirtualRegs({reg});
        }

        loadVirtualRegs({i.src.get()});

        os_ << "mov ";

        wordType_ = lir::WordType::Qword;
        bool shouldWrap = utils::isa<lir::Register>(i.src.get());
        if (shouldWrap) {
            os_ << '[';
        }
        visitAddress(i.src.get());
        if (shouldWrap) {
            os_ << ']';
        }

        os_ << ", ";

        wordType_ = i.itemSize();
        visitRegister(i.src.get());
        os_ << '\n';
    }

    void visitCmpInstr(const lir::CmpInstr& i) override {
        loadVirtualRegs({i.lhs.get(), i.rhs.get()});
        allocatePhysicalRegIfVirtual(i.dest.get());

        os_ << "cmp ";
        wordType_ = i.itemSize;
        visitRegister(i.lhs.get());
        os_ << ", ";
        visitRegister(i.rhs.get());
        os_ << '\n';

        os_ << "set";
        switch (i.cond) {
            case lir::CmpInstr::Cond::Eq:
                os_ << "e";
                break;
            case lir::CmpInstr::Cond::Ne:
                os_ << "ne";
                break;
            case lir::CmpInstr::Cond::Lt:
                os_ << "l";
                break;
            case lir::CmpInstr::Cond::Le:
                os_ << "le";
                break;
            case lir::CmpInstr::Cond::Gt:
                os_ << "g";
                break;
            case lir::CmpInstr::Cond::Ge:
                os_ << "ge";
                break;
        }
        os_ << ' ';
        wordType_ = lir::WordType::Byte;
        visitRegister(i.dest.get());
        os_ << '\n';
        storeIfVirtualRegister(i.dest.get());
    }

    void visitAddInstr(const lir::AddInstr& i) override {
        wordType_ = lir::WordType::Dword;
        loadVirtualRegs({i.lhsDest.get()});

        bool rhsIsRegister = utils::isa<lir::Register>(i.rhs.get());
        if (rhsIsRegister) {
            loadVirtualRegs({static_cast<const lir::Register*>(i.rhs.get())});
        }

        os_ << "add ";
        visitRegister(i.lhsDest.get());
        os_ << ", ";

        if (rhsIsRegister) {
            visitRegister(static_cast<const lir::Register*>(i.rhs.get()));
        } else {
            assert(utils::isa<lir::Immediate>(i.rhs.get()));
            os_ << static_cast<const lir::Immediate*>(i.rhs.get())->value();
        }
        os_ << '\n';

        storeIfVirtualRegister(i.lhsDest.get());
    }

    void visitSubInstr(const lir::SubInstr& i) override {
        wordType_ = lir::WordType::Dword;
        loadVirtualRegs({i.lhsDest.get()});

        bool rhsIsRegister = utils::isa<lir::Register>(i.rhs.get());
        if (rhsIsRegister) {
            loadVirtualRegs({static_cast<const lir::Register*>(i.rhs.get())});
        }

        os_ << "sub ";
        visitRegister(i.lhsDest.get());
        os_ << ", ";

        if (rhsIsRegister) {
            visitRegister(static_cast<const lir::Register*>(i.rhs.get()));
        } else {
            assert(utils::isa<lir::Immediate>(i.rhs.get()));
            os_ << static_cast<const lir::Immediate*>(i.rhs.get())->value();
        }
        os_ << '\n';

        storeIfVirtualRegister(i.lhsDest.get());
    }

    void visitMulInstr(const lir::MulInstr& i) override {
        wordType_ = lir::WordType::Dword;
        loadVirtualRegs({i.lhsDest.get(), i.rhs.get()});

        os_ << "imul ";
        visitRegister(i.lhsDest.get());
        os_ << ", ";
        visitRegister(i.rhs.get());
        os_ << '\n';

        storeIfVirtualRegister(i.lhsDest.get());
    }

    void visitDivInstr(const lir::DivInstr& i) override {
        loadVirtualRegs({i.rhs.get()});
        os_ << "div ";
        visitRegister(i.rhs.get());
        os_ << '\n';
    }

    void visitRetInstr(const lir::RetInstr&) override {
        os_ << "add rsp, "
            << lir::wordTypeToSize(lir::WordType::Qword) * virtualsCount_
            << '\n';
        os_ << "ret\n";
    }

    void visitJmpInstr(const lir::JmpInstr& i) override {
        if (!i.conditional()) {
            os_ << "jmp .bb" << i.dest()->idx() << "\n";
            return;
        }

        wordType_ = lir::WordType::Byte;
        loadVirtualRegs({i.cond->first.get()});
        os_ << "test ";
        visitRegister(i.cond->first.get());
        os_ << ", ";
        visitRegister(i.cond->first.get());
        os_ << '\n';

        if (i.invertedCond()) {
            os_ << "jz .bb" << i.dest()->idx() << "\n";
        } else {
            os_ << "jnz .bb" << i.dest()->idx() << "\n";
        }
    }

public:
    Generator(std::ostream& os) : os_{os} {}

    void visit(const lir::Module& mod) override {
        bool first = false;

        os_ << "bits 64\n";
        os_ << "section .text\n";
        os_ << "extern runtime_alloc\n";
        
        for (auto&& fn : mod.functions()) {
            if (!first) {
                os_ << '\n';
            }
            first = false;
            visit(fn);
        }

        if (!mod.globals().empty()) {
            if (!first) {
                os_ << '\n';
            }
            os_ << "section .data\n";
        }
        for (auto&& [name, global] : mod.globals()) {
            visitGlobalDecl(name, global);
        }
    }
};
}  // namespace

void codegen(std::ostream& os, const lir::Module& mod) {
    Generator generator(os);
    generator.visit(mod);
}
}  // namespace backend
