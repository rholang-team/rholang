#pragma once

#include <list>

#include "compiler/ir/bb.hpp"
#include "compiler/ir/type.hpp"

namespace ir {
class Function {
    Type* type_;
    std::list<BasicBlock> bbs_;
};
}  // namespace ir
