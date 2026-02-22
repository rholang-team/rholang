#pragma once

#include <list>
#include <memory>

namespace ir {
class Instr;

class BasicBlock {
    std::list<std::shared_ptr<Instr>> instrs_;
};
}  // namespace ir
