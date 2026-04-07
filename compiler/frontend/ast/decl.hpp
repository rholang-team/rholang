#pragma once

#include <string>
#include <utility>
#include <vector>

#include "compiler/frontend/ast/expr.hpp"
#include "compiler/frontend/ast/stmt.hpp"
#include "compiler/frontend/lex/span.hpp"
#include "compiler/frontend/type.hpp"

namespace frontend::ast {
struct VarDecl {
    lex::WithSpan<std::string> name;
    lex::WithSpan<std::shared_ptr<Type>> type;
    std::shared_ptr<Expr> value;

    VarDecl(lex::WithSpan<std::string> name,
            lex::WithSpan<std::shared_ptr<Type>> type,
            std::shared_ptr<Expr> value)
        : name{std::move(name)}, type{type}, value{value} {}
};

struct FunctionDecl {
    lex::WithSpan<std::string> name;
    std::vector<std::string> paramNames;
    std::vector<lex::WithSpan<std::shared_ptr<Type>>> paramTypes;
    lex::WithSpan<std::shared_ptr<Type>> rettype;
    CompoundStmt body;

    FunctionDecl(lex::WithSpan<std::string> name,
                 std::vector<std::string> paramNames,
                 std::vector<lex::WithSpan<std::shared_ptr<Type>>> paramTypes,
                 lex::WithSpan<std::shared_ptr<Type>> rettype,
                 CompoundStmt body)
        : name{std::move(name)},
          paramNames{std::move(paramNames)},
          paramTypes{std::move(paramTypes)},
          rettype{std::move(rettype)},
          body{std::move(body)} {}

    FunctionType type() const;

    bool isInstanceMethod() const;
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
    std::unordered_map<std::string, std::shared_ptr<FunctionDecl>> methods;

    StructDecl(
        lex::WithSpan<std::string> name,
        std::vector<Field> fields,
        std::unordered_map<std::string, std::shared_ptr<FunctionDecl>> methods)
        : name{std::move(name)},
          fields{std::move(fields)},
          methods{std::move(methods)} {}

    StructType type() const;
};
}  // namespace frontend::ast
