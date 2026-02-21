#pragma once

#include <memory>

#include "compiler/ir/type.hpp"

namespace ir {
struct Value {
    std::shared_ptr<Type> type;

    virtual ~Value() = default;

    explicit Value(std::shared_ptr<Type> type) : type{type} {}
};

struct IntImm final : public Value {
    int value;

    IntImm(int value) : Value{} {}
};
}  // namespace ir
