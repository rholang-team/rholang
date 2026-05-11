#pragma once

#include "compiler/lir/module.hpp"

namespace backend {
std::string structMapName(std::string_view s);

void codegen(std::ostream& os, const lir::Module& module);
}  // namespace backend
