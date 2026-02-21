#pragma once

#include <list>
#include <memory>
#include <vector>

#include "compiler/ir/bb.hpp"
#include "compiler/ir/type.hpp"

namespace ir {
class Function {
    std::vector<std::shared_ptr<Type>> params;
    std::list<BasicBlock> bbs;
};
}  // namespace ir
