#pragma once

#include "compiler/lir/module.hpp"

namespace backend {
void codegen(std::ostream& os, const lir::Module& module);
}  // namespace backend
