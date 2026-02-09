#include "frontend/ast/decl.hpp"

namespace frontend::ast {
FunctionType FunctionDecl::type() const {
    std::vector<std::shared_ptr<Type>> paramTypes;

    for (const auto& [_, t] : params) {
        paramTypes.emplace_back(t.value);
    }

    return FunctionType{std::move(paramTypes), rettype.value};
}
}  // namespace frontend::ast
