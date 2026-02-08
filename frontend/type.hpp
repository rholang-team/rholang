#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace frontend {
struct Type {
    virtual ~Type() = default;
};

struct PrimitiveType final : public Type {
    enum class Primitive {
        Void,
        Bool,
        Int,
    };

    Primitive kind;

    explicit PrimitiveType(Primitive kind) : kind{kind} {}

    static const std::shared_ptr<Type> voidType;
    static const std::shared_ptr<Type> boolType;
    static const std::shared_ptr<Type> intType;
};

struct FunctionType final : public Type {
    std::vector<std::shared_ptr<Type>> params;
    std::shared_ptr<Type> rettype;

    FunctionType() = default;
    FunctionType(std::vector<std::shared_ptr<Type>> params, std::shared_ptr<Type> rettype)
        : params{std::move(params)}, rettype{rettype} {}
};

struct TypeRef final : public Type {
    std::string name;

    template <std::constructible_from<std::string> S>
    explicit TypeRef(S&& s) : name{std::forward<S>(s)} {}
};
}  // namespace frontend

std::ostream& operator<<(std::ostream& os, const frontend::Type& ty);
std::ostream& operator<<(std::ostream& os, const frontend::PrimitiveType& ty);
std::ostream& operator<<(std::ostream& os, const frontend::TypeRef& ty);
std::ostream& operator<<(std::ostream& os, const frontend::FunctionType& ty);
