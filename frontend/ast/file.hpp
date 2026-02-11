#pragma once

#include <string>
#include <unordered_map>

#include "frontend/ast/decl.hpp"

namespace frontend::ast {
struct File {
    std::string_view input;
    std::unordered_map<std::string, ast::VarDecl> globals;
    std::unordered_map<std::string, ast::FunctionDecl> functions;
    std::unordered_map<std::string, ast::StructDecl> structs;

    File() = default;

    File(std::string_view input,
         std::unordered_map<std::string, ast::VarDecl> globals,
         std::unordered_map<std::string, ast::FunctionDecl> functions,
         std::unordered_map<std::string, ast::StructDecl> structs)
        : input{input},
          globals{std::move(globals)},
          functions{std::move(functions)},
          structs{std::move(structs)} {}
};
}  // namespace frontend::ast
