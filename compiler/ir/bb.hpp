#pragma once

#include <list>
#include <memory>

#include "compiler/ir/instr.hpp"

namespace ir {
class BasicBlock {
    std::list<std::shared_ptr<Instr>> instrs_;
};
}  // namespace ir
