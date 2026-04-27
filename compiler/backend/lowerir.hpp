#pragma once

#include "compiler/ir/module.hpp"
#include "compiler/lir/module.hpp"

namespace backend {
lir::Module lowerIr(const ir::Module& mod);
}
