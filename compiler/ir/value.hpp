#pragma once

#include <memory>

#include "compiler/ir/context.hpp"
#include "compiler/ir/function.hpp"
#include "compiler/ir/type.hpp"

namespace ir {
class Value {
    Type* type_;

protected:
    explicit Value(Type* type) : type_{type} {}

public:
    virtual ~Value() = default;

    Type* type() const;
};

class IntImm final : public Value {
    int value_;

    IntImm(IntType* ty, int value) : Value{ty}, value_{value} {}

public:
    static std::shared_ptr<IntImm> create(Context& ctx, int value);

    int value() const;
};

class BoolImm final : public Value {
    bool value_;

    BoolImm(BoolType* ty, bool value) : Value{ty}, value_{value} {}

public:
    static std::shared_ptr<BoolImm> create(Context& ctx, bool value);

    bool value() const;
};

class FnArgRef final : public Value {
    unsigned idx_;

    FnArgRef(Type* ty, unsigned idx) : Value{ty}, idx_{idx} {}

public:
    static std::shared_ptr<FnArgRef> create(Function* fn, unsigned idx);

    unsigned idx() const;
};
}  // namespace ir
