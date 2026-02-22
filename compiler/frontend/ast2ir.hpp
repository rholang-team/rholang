#pragma once

#include "compiler/frontend/translationunit.hpp"
#include "compiler/ir/module.hpp"

namespace frontend::ast2ir {
ir::Module translate(ir::Context& ctx, TranslationUnit& tu);
}  // namespace frontend::ast2ir
