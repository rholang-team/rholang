#pragma once

#include <list>
#include <memory>

namespace ir {
class Instr;

class BasicBlock {
    using Instrs = std::list<std::shared_ptr<Instr>>;

    Instrs instrs_;
    bool hasTerminator_ = false;

public:
    void addInstr(std::shared_ptr<Instr> i);

    Instrs& instrs() & {
        return instrs_;
    }
    const Instrs& instrs() const& {
        return instrs_;
    }

    bool hasTerminator() const {
        return hasTerminator_;
    }
};
}  // namespace ir
