#pragma once

#include <string>
#include <unordered_map>

#include "frontend/ast/decl.hpp"

namespace frontend {
struct TranslationUnit {
    std::unordered_map<std::string, ast::VarDecl> globals;
    std::unordered_map<std::string, ast::FunctionDecl> functions;
    std::unordered_map<std::string, StructType> structs;
};
}  // namespace frontend
