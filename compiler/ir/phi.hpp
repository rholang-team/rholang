#pragma once

#include <cassert>
#include <unordered_map>

#include "compiler/ir/value.hpp"

namespace ir {
class BasicBlock;

class PhiNode final : public ir::Value {
    std::unordered_map<const BasicBlock*, std::shared_ptr<ir::Value>> values_;

public:
    PhiNode(std::unordered_map<const BasicBlock*, std::shared_ptr<ir::Value>>
                values);

    std::optional<std::shared_ptr<ir::Value>> valueFrom(
        const BasicBlock* bb) const;
};
}  // namespace ir
