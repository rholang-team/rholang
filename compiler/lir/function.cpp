#include "compiler/lir/function.hpp"

namespace lir {
void Function::addBb(std::unique_ptr<BasicBlock> bb) {
    bbs_.emplace_back(std::move(bb));
}
}  // namespace lir
