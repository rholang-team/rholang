#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "frontend/ast/decl.hpp"

namespace frontend {
struct TranslationUnit {
    std::unordered_map<std::string, std::unique_ptr<ast::Decl>> decls;
};
}  // namespace frontend
