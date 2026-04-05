#include "compiler/ir/bb.hpp"

#include "compiler/ir/instr.hpp"

namespace ir {
void BasicBlock::addInstr(std::shared_ptr<Instr> i) {
    instrs_.push_back(i);
    if (i->isTerminator()) {
        hasTerminator_ = true;
    }
}

bool BasicBlock::operator==(const BasicBlock& that) const {
    return std::ranges::equal(
        instrs_,
        that.instrs_,
        [](const std::shared_ptr<Instr>& a, const std::shared_ptr<Instr>& b) {
            return *a == *b;
        });
}
}  // namespace ir
