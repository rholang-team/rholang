#pragma once

#include <string>
#include <utility>
#include <vector>

#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/type.hpp"

namespace frontend::ast {
struct Decl : public pretty::PrettyPrintable {
    virtual ~Decl() = default;
};

struct VarDecl final : public Decl {
    std::string name;
    std::shared_ptr<Type> type;
    std::unique_ptr<Expr> value;

    template <typename S>
        requires std::convertible_to<std::string, S> || std::constructible_from<std::string, S>
    VarDecl(S&& s, std::shared_ptr<Type> ty, std::unique_ptr<Expr> v)
        : name{std::forward<S>(s)}, type{ty}, value{std::move(v)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct FunctionDecl final : public Decl {
    std::string name;
    std::vector<std::string> paramNames;
    std::shared_ptr<FunctionType> type;
    CompoundStmt body;

    template <typename S, typename S2>
        requires(std::convertible_to<std::string, S> || std::constructible_from<std::string, S>) &&
                    (std::convertible_to<std::string, S2> ||
                     std::constructible_from<std::string, S2>)
    FunctionDecl(S&& name,
                 std::vector<std::pair<S2, std::shared_ptr<Type>>> params,
                 std::shared_ptr<Type> rettype,
                 CompoundStmt body)
        : name{std::forward<S>(name)}, body{std::move(body)} {
        std::vector<std::string> names;
        std::vector<std::shared_ptr<Type>> types;

        for (auto& [name, ty] : params) {
            names.emplace_back(std::move(name));
            types.emplace_back(ty);
        }

        paramNames = std::move(names);
        type = std::make_shared<FunctionType>(std::move(types), rettype);
    }

    template <typename S>
        requires std::convertible_to<std::string, S> || std::constructible_from<std::string, S>
    FunctionDecl(S&& name,
                 std::vector<std::string> paramNames,
                 std::vector<std::shared_ptr<Type>> paramTypes,
                 std::shared_ptr<Type> rettype,
                 CompoundStmt body)
        : name{std::forward<S>(name)},
          paramNames{std::move(paramNames)},
          type{std::make_shared<FunctionType>(paramTypes, rettype)},
          body{std::move(body)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};
}  // namespace frontend::ast
