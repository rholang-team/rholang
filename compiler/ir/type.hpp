#pragma once

#include <span>
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

    Type* underlying() const;
};

class StructType final : public Type {
    std::span<Type*> fields_;

    explicit StructType(std::span<Type*> fields) : fields_{fields} {}

public:
    static StructType* get(Context& ctx, std::span<Type*> fields);

    std::span<Type*> fields();
    std::span<Type*> fields() const;
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

    Type* rettype() const;
    std::span<Type*> params() const;
};
}  // namespace ir
