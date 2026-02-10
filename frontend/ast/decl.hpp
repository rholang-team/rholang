#pragma once

#include <string>
#include <utility>
#include <vector>

#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/lex/span.hpp"
#include "frontend/type.hpp"

namespace frontend::ast {
struct Decl {
    lex::WithSpan<std::string> name;

    Decl(lex::WithSpan<std::string> name) : name{std::move(name)} {}

    virtual ~Decl() = default;
};

struct VarDecl final : public Decl {
    lex::WithSpan<std::shared_ptr<Type>> type;
    std::optional<std::unique_ptr<Expr>> value;

    VarDecl(lex::WithSpan<std::string> name,
            lex::WithSpan<std::shared_ptr<Type>> type,
            std::optional<std::unique_ptr<Expr>> value)
        : Decl{std::move(name)}, type{type}, value{std::move(value)} {}
};

struct FunctionDecl final : public Decl {
    std::vector<std::pair<lex::WithSpan<std::string>, lex::WithSpan<std::shared_ptr<Type>>>> params;
    lex::WithSpan<std::shared_ptr<Type>> rettype;
    CompoundStmt body;

    FunctionDecl(
        lex::WithSpan<std::string> name,
        std::vector<std::pair<lex::WithSpan<std::string>, lex::WithSpan<std::shared_ptr<Type>>>>
            params,
        lex::WithSpan<std::shared_ptr<Type>> rettype,
        CompoundStmt body)
        : Decl{std::move(name)},
          params{std::move(params)},
          rettype{std::move(rettype)},
          body{std::move(body)} {}

    FunctionType type() const;
};
}  // namespace frontend::ast
