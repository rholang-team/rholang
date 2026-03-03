#include "compiler/ir/bb.hpp"

#include "compiler/ir/instr.hpp"

namespace ir {
void BasicBlock::addInstr(std::shared_ptr<Instr> i) {
    instrs_.push_back(i);
    if (i->isTerminator()) {
        hasTerminator_ = true;
    }
}
}  // namespace ir
