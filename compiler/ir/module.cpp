#include "compiler/ir/module.hpp"

namespace ir {
void Module::addFunction(std::unique_ptr<Function> fn) {
    functions_.emplace(fn->name(), std::move(fn));
}
}  // namespace ir
