#include "compiler/ir/module.hpp"

namespace ir {
void Module::addSignature(std::unique_ptr<FunctionSignature> fn) {
    signatures_.emplace(fn->name(), std::move(fn));
}

void Module::addFunction(std::unique_ptr<Function> fn) {
    functions_.emplace(fn->signature()->name(), std::move(fn));
}
}  // namespace ir
