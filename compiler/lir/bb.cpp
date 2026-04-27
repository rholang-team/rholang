#include "compiler/lir/bb.hpp"

#include "compiler/lir/instr.hpp"

namespace lir {
void BasicBlock::addInstr(std::unique_ptr<Instr> i) {
    instrs_.emplace_back(std::move(i));
}
}  // namespace lir
