#pragma once

#include "frontend/ast/file.hpp"
#include "frontend/translationunit.hpp"

namespace frontend {
TranslationUnit runSema(ast::File file);
}  // namespace frontend
