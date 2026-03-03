#pragma once

#include <format>
#include <ostream>
#include <span>
#include <utility>

namespace ir {
class Context;

struct VoidType;
struct BoolType;
struct IntType;

struct Type {
    static VoidType* getVoidTy(Context& c);
    static BoolType* getBoolTy(Context& c);
    static IntType* getIntTy(Context& c);

    virtual ~Type() = default;
};

struct VoidType final : public Type {};
struct BoolType final : public Type {};
struct IntType final : public Type {};

class PointerType final : public Type {
    Type* underlying_;

    explicit PointerType(Type* underlying) : underlying_{underlying} {}

public:
    static PointerType* get(Context& ctx, Type* underlying);

    Type* underlying() const {
        return underlying_;
    }
};

class StructType final : public Type {
    std::span<Type*> fields_;

    explicit StructType(std::span<Type*> fields) : fields_{fields} {}

public:
    static StructType* get(Context& ctx, std::span<Type*> fields);

    std::span<Type*> fields() {
        return fields_;
    }
    std::span<Type*> fields() const {
        return fields_;
    }
};

class FunctionType final : public Type {
    Type* rettype_;
    std::span<Type*> params_;

    FunctionType(Type* rettype, std::span<Type*> params)
        : rettype_{rettype}, params_{params} {}

public:
    static FunctionType* get(Context& ctx,
                             Type* rettype,
                             std::span<Type*> params);

    Type* rettype() const {
        return rettype_;
    }
    std::span<Type*> params() const {
        return params_;
    }
};
}  // namespace ir

#define DECL_FORMAT(T)                                   \
    template <>                                          \
    struct std::formatter<T> {                           \
        template <typename C>                            \
        constexpr auto parse(C& ctx) {                   \
            return ctx.begin();                          \
        }                                                \
                                                         \
        template <typename C>                            \
        C::iterator format(const T& type, C& ctx) const; \
    };

#define IMPL_FORMAT(T)                                                    \
    template <typename C>                                                 \
    C::iterator std::formatter<T>::format([[maybe_unused]] const T& type, \
                                          C& ctx) const

DECL_FORMAT(ir::Type)

DECL_FORMAT(ir::VoidType)
IMPL_FORMAT(ir::VoidType) {
    return std::format_to(ctx.out(), "void");
}

DECL_FORMAT(ir::IntType)
IMPL_FORMAT(ir::IntType) {
    return std::format_to(ctx.out(), "int");
}

DECL_FORMAT(ir::BoolType)
IMPL_FORMAT(ir::BoolType) {
    return std::format_to(ctx.out(), "bool");
}

DECL_FORMAT(ir::PointerType)
IMPL_FORMAT(ir::PointerType) {
    return std::format_to(ctx.out(), "*{}", *type.underlying());
}

DECL_FORMAT(ir::StructType)
IMPL_FORMAT(ir::StructType) {
    std::format_to(ctx.out(), "struct{{");

    bool first = true;
    for (ir::Type* field : type.fields()) {
        if (!first) {
            std::format_to(ctx.out(), ", ");
        }
        first = false;
        std::format_to(ctx.out(), "{}", *field);
    }
    return std::format_to(ctx.out(), "}}");
}

DECL_FORMAT(ir::FunctionType)
IMPL_FORMAT(ir::FunctionType) {
    // I hope this is not needed

    std::unreachable();
    (void)ctx;
}

IMPL_FORMAT(ir::Type) {
    if (const ir::VoidType* voidType =
            dynamic_cast<const ir::VoidType*>(&type)) {
        return std::format_to(ctx.out(), "{}", *voidType);
    } else if (const ir::IntType* intType =
                   dynamic_cast<const ir::IntType*>(&type)) {
        return std::format_to(ctx.out(), "{}", *intType);
    } else if (const ir::BoolType* boolType =
                   dynamic_cast<const ir::BoolType*>(&type)) {
        return std::format_to(ctx.out(), "{}", *boolType);
    } else if (const ir::PointerType* ptrType =
                   dynamic_cast<const ir::PointerType*>(&type)) {
        return std::format_to(ctx.out(), "{}", *ptrType);
    } else if (const ir::StructType* structType =
                   dynamic_cast<const ir::StructType*>(&type)) {
        return std::format_to(ctx.out(), "{}", *structType);
    } else if (const ir::FunctionType* fnType =
                   dynamic_cast<const ir::FunctionType*>(&type)) {
        return std::format_to(ctx.out(), "{}", *fnType);
    }

    std::unreachable();
}

#undef DECL_FORMAT
#undef IMPL_FORMAT

std::ostream& operator<<(std::ostream& os, const ir::Type& ty);
std::ostream& operator<<(std::ostream& os, const ir::VoidType& ty);
std::ostream& operator<<(std::ostream& os, const ir::IntType& ty);
std::ostream& operator<<(std::ostream& os, const ir::BoolType& ty);
std::ostream& operator<<(std::ostream& os, const ir::PointerType& ty);
std::ostream& operator<<(std::ostream& os, const ir::StructType& ty);
std::ostream& operator<<(std::ostream& os, const ir::FunctionType& ty);
