#pragma once

#include "compiler/frontend/ast/file.hpp"
#include "compiler/frontend/translationunit.hpp"

namespace frontend {
TranslationUnit runSema(ast::File file);
}  // namespace frontend
