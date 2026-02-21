#pragma once

#include "compiler/ir/context.hpp"
#include "compiler/ir/type.hpp"

namespace ir {
struct Value {
    Type* type;

    virtual ~Value() = default;

    explicit Value(Type* type) : type{type} {}
};

struct IntImm final : public Value {
    int value;

    IntImm(Context& ctx, int value) : Value{ctx.getIntTy()}, value{value} {}
};

struct BoolImm final : public Value {
    bool value;

    BoolImm(Context& ctx, bool value) : Value{ctx.getBoolTy()}, value{value} {}
};
}  // namespace ir
