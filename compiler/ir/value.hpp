#pragma once

#include <memory>

#include "compiler/ir/context.hpp"
#include "compiler/ir/type.hpp"

namespace ir {
class Value {
    Type* type_;

public:
    virtual ~Value() = default;

    explicit Value(Type* type) : type_{type} {}
    Type* type() const;

    bool operator==(const Value& that) const;
};

class IntImm final : public Value {
    int value_;

    IntImm(IntType* ty, int value) : Value{ty}, value_{value} {}

public:
    static std::shared_ptr<IntImm> get(Context& ctx, int value);

    int value() const;

    bool operator==(const IntImm& that) const;
};

class BoolImm final : public Value {
    bool value_;

    BoolImm(BoolType* ty, bool value) : Value{ty}, value_{value} {}

public:
    static std::shared_ptr<BoolImm> get(Context& ctx, bool value);

    bool value() const;

    bool operator==(const BoolImm& that) const;
};

class TmpVar final : public Value {
    size_t idx_;

    TmpVar(Type* type, size_t idx) : Value{type}, idx_{idx} {}

public:
    static std::shared_ptr<TmpVar> get(Context& ctx, Type* ty);

    size_t idx() const;

    bool operator==(const TmpVar& that) const;
};
}  // namespace ir
