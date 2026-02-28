#pragma once

#include <list>
#include <memory>

namespace ir {
class Instr;

class BasicBlock {
    using Instrs = std::list<std::shared_ptr<Instr>>;

    Instrs instrs_;

public:
    void addInstr(std::shared_ptr<Instr>);

    const Instrs& instrs() const;
};
}  // namespace ir
