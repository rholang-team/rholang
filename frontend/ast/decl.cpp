#include "frontend/ast/decl.hpp"

namespace frontend::ast {
FunctionType FunctionDecl::type() const {
    std::vector<std::shared_ptr<Type>> paramTypes;

    for (const auto& t : this->paramTypes) {
        paramTypes.emplace_back(t.value);
    }

    return FunctionType{std::move(paramTypes), rettype.value};
}

bool FunctionDecl::isInstanceMethod() const {
    return !paramNames.empty() && paramNames[0] == "self";
}

StructType StructDecl::type() const {
    std::vector<StructType::Field> fields;

    for (const auto& field : this->fields) {
        fields.emplace_back(field.name.value, field.type.value);
    }

    return StructType{name.value, fields};
}
}  // namespace frontend::ast
