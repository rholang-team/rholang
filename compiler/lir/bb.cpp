#include "compiler/lir/bb.hpp"

#include "compiler/lir/instr.hpp"
#include "utils/match.hpp"

namespace lir {
BasicBlock::BasicBlock(size_t idx) : idx_{idx} {}

BasicBlock::~BasicBlock() = default;

void BasicBlock::addInstr(std::unique_ptr<Instr> i) {
    instrs_.emplace_back(std::move(i));
}

std::optional<const lir::JmpInstr*> BasicBlock::findJmp() const {
    auto it = std::ranges::find_if(*this, [](const auto& i) {
        return utils::isa<JmpInstr>(i.get());
    });

    if (it == end()) {
        return std::nullopt;
    }

    return static_cast<const JmpInstr*>(it->get());
}

bool BasicBlock::hasReturn() const {
    return std::ranges::find_if(*this, [](const auto& i) {
               return utils::isa<RetInstr>(i.get());
           }) != end();
}
}  // namespace lir
