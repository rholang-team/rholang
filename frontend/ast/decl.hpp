#pragma once

#include <string>
#include <utility>
#include <vector>

#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/lex/span.hpp"
#include "frontend/type.hpp"

namespace frontend::ast {
struct Decl : public pretty::PrettyPrintable {
    virtual ~Decl() = default;
};

struct VarDecl final : public Decl {
    lex::WithSpan<std::string> name;
    lex::WithSpan<std::shared_ptr<Type>> type;
    std::unique_ptr<Expr> value;

    VarDecl(lex::WithSpan<std::string> name,
            lex::WithSpan<std::shared_ptr<Type>> type,
            std::unique_ptr<Expr> value)
        : name{std::move(name)}, type{type}, value{std::move(value)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct FunctionDecl final : public Decl {
    lex::WithSpan<std::string> name;
    std::vector<std::pair<lex::WithSpan<std::string>, lex::WithSpan<std::shared_ptr<Type>>>> params;
    lex::WithSpan<std::shared_ptr<Type>> rettype;
    CompoundStmt body;

    FunctionDecl(
        lex::WithSpan<std::string> name,
        std::vector<std::pair<lex::WithSpan<std::string>, lex::WithSpan<std::shared_ptr<Type>>>>
            params,
        lex::WithSpan<std::shared_ptr<Type>> rettype,
        CompoundStmt body)
        : name{std::move(name)},
          params{std::move(params)},
          rettype{std::move(rettype)},
          body{std::move(body)} {}

    FunctionType type() const;

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};
}  // namespace frontend::ast
