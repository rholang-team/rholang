#include "compiler/lir/module.hpp"

#include "compiler/lir/instr.hpp"

namespace lir {
Module::~Module() = default;
Module::Module() = default;
Module::Module(Module&&) = default;

void Module::addFunction(Function fn) {
    functions_.emplace_back(std::move(fn));
}
}  // namespace lir
