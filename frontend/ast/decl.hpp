#pragma once

#include <string>
#include <utility>
#include <vector>

#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/lex/span.hpp"
#include "frontend/type.hpp"

namespace frontend::ast {
struct VarDecl {
    lex::WithSpan<std::string> name;
    lex::WithSpan<std::shared_ptr<Type>> type;
    std::optional<std::shared_ptr<Expr>> value;

    VarDecl(lex::WithSpan<std::string> name,
            lex::WithSpan<std::shared_ptr<Type>> type,
            std::optional<std::shared_ptr<Expr>> value)
        : name{std::move(name)}, type{type}, value{std::move(value)} {}
};

struct FunctionDecl {
    using Param = std::pair<lex::WithSpan<std::string>,
                            lex::WithSpan<std::shared_ptr<Type>>>;

    lex::WithSpan<std::string> name;
    std::vector<lex::WithSpan<std::string>> paramNames;
    std::vector<lex::WithSpan<std::shared_ptr<Type>>> paramTypes;
    lex::WithSpan<std::shared_ptr<Type>> rettype;
    CompoundStmt body;

    FunctionDecl(lex::WithSpan<std::string> name,
                 std::vector<lex::WithSpan<std::string>> paramNames,
                 std::vector<lex::WithSpan<std::shared_ptr<Type>>> paramTypes,
                 lex::WithSpan<std::shared_ptr<Type>> rettype,
                 CompoundStmt body)
        : name{std::move(name)},
          paramNames{std::move(paramNames)},
          paramTypes{std::move(paramTypes)},
          rettype{std::move(rettype)},
          body{std::move(body)} {}

    FunctionType type() const;
};

struct StructDecl {
    struct Field {
        lex::WithSpan<std::string> name;
        lex::WithSpan<std::shared_ptr<Type>> type;

        Field(lex::WithSpan<std::string> name,
              lex::WithSpan<std::shared_ptr<Type>> type)
            : name{std::move(name)}, type{type} {}
    };

    lex::WithSpan<std::string> name;
    std::vector<Field> fields;
    std::unordered_map<std::string, FunctionDecl> methods;

    StructDecl(lex::WithSpan<std::string> name,
               std::vector<Field> fields,
               std::unordered_map<std::string, FunctionDecl> methods)
        : name{std::move(name)},
          fields{std::move(fields)},
          methods{std::move(methods)} {}
};
}  // namespace frontend::ast
