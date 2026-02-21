#pragma once

#include <memory>

#include "compiler/ir/context.hpp"
#include "compiler/ir/value.hpp"

// TODO

namespace ir {
class Builder {
    Context& ctx_;

public:
    explicit Builder(Context& ctx) : ctx_{ctx} {}

    std::shared_ptr<IntImm> createImm(int value);
};
}  // namespace ir
