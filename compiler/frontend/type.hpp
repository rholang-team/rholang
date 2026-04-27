#pragma once

#include <format>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "utils/match.hpp"

namespace frontend {
struct Type {
    virtual ~Type() = default;

    bool operator==(const Type& that) const;
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

    bool operator==(const PrimitiveType& that) const;
};

struct FunctionType final : public Type {
    std::vector<std::shared_ptr<Type>> params;
    std::shared_ptr<Type> rettype;

    FunctionType() = default;
    FunctionType(std::vector<std::shared_ptr<Type>> params,
                 std::shared_ptr<Type> rettype)
        : params{std::move(params)}, rettype{rettype} {}

    bool operator==(const FunctionType& that) const;
};

struct TypeRef final : public Type {
    std::string name;

    template <typename S>
        requires std::convertible_to<std::string, S> ||
                 std::constructible_from<std::string, S>
    explicit TypeRef(S&& s) : name{std::forward<S>(s)} {}

    bool operator==(const TypeRef& that) const;
};

struct StructType final : public Type {
    struct Field {
        std::string name;
        std::shared_ptr<Type> type;

        template <typename S>
            requires std::convertible_to<std::string, S> ||
                         std::constructible_from<std::string, S>
        Field(S&& name, std::shared_ptr<Type> type)
            : name{std::forward<S>(name)}, type{type} {}

        bool operator==(const Field& that) const = default;
    };

    std::string name;
    std::vector<Field> fields;

    template <typename S>
        requires std::convertible_to<std::string, S> ||
                     std::constructible_from<std::string, S>
    StructType(S&& name, std::vector<Field> fields)
        : name{std::forward<S>(name)}, fields{std::move(fields)} {}

    bool operator==(const StructType& that) const;
};
}  // namespace frontend

template <>
struct std::formatter<frontend::Type> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename Ctx>
    Ctx::iterator format(const frontend::Type& type, Ctx& ctx) const;
};

template <>
struct std::formatter<frontend::PrimitiveType> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename Ctx>
    Ctx::iterator format(const frontend::PrimitiveType& type, Ctx& ctx) const {
        switch (type.kind) {
            case frontend::PrimitiveType::Primitive::Void:
                return std::format_to(ctx.out(), "void");
            case frontend::PrimitiveType::Primitive::Bool:
                return std::format_to(ctx.out(), "bool");
            case frontend::PrimitiveType::Primitive::Int:
                return std::format_to(ctx.out(), "int");
        }
    }
};

template <>
struct std::formatter<frontend::FunctionType> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename Ctx>
    Ctx::iterator format(const frontend::FunctionType& type, Ctx& ctx) const {
        std::format_to(ctx.out(), "(");

        bool first = true;
        for (const auto& p : type.params) {
            if (!first) {
                std::ranges::copy(", ", ctx.out());
            }
            first = false;
            std::format_to(ctx.out(), "{}", *p);
        }

        return std::format_to(ctx.out(), ") -> {}", *type.rettype);
    }
};

template <>
struct std::formatter<frontend::TypeRef> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename Ctx>
    Ctx::iterator format(const frontend::TypeRef& type, Ctx& ctx) const {
        std::ranges::copy(type.name, ctx.out());
        return ctx.out();
    }
};

template <>
struct std::formatter<frontend::StructType> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename Ctx>
    Ctx::iterator format(const frontend::StructType& type, Ctx& ctx) const {
        std::format_to(ctx.out(), "struct {} {{", type.name);
        bool first = true;
        for (auto& field : type.fields) {
            if (!first)
                std::ranges::copy(", ", ctx.out());
            first = false;
            std::format_to(ctx.out(), "{} {}", field.name, *field.type);
        }
        std::format_to(ctx.out(), "}}");
        return ctx.out();
    }
};

template <typename Ctx>
Ctx::iterator std::formatter<frontend::Type>::format(const frontend::Type& type,
                                                     Ctx& ctx) const {
    if (utils::isa<frontend::PrimitiveType>(type)) {
        return std::format_to(
            ctx.out(),
            "{}",
            dynamic_cast<const frontend::PrimitiveType&>(type));
    }
    if (utils::isa<frontend::FunctionType>(type)) {
        return std::format_to(
            ctx.out(),
            "{}",
            dynamic_cast<const frontend::FunctionType&>(type));
    }
    if (utils::isa<frontend::TypeRef>(type)) {
        return std::format_to(ctx.out(),
                              "{}",
                              dynamic_cast<const frontend::TypeRef&>(type));
    }
    if (utils::isa<frontend::StructType>(type)) {
        return std::format_to(ctx.out(),
                              "{}",
                              dynamic_cast<const frontend::StructType&>(type));
    }

    std::unreachable();
}

std::ostream& operator<<(std::ostream& os, const frontend::Type& ty);
std::ostream& operator<<(std::ostream& os, const frontend::PrimitiveType& ty);
std::ostream& operator<<(std::ostream& os, const frontend::TypeRef& ty);
std::ostream& operator<<(std::ostream& os, const frontend::FunctionType& ty);
