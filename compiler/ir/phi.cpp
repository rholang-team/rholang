#include "compiler/ir/phi.hpp"

#include <algorithm>

namespace ir {
PhiNode::PhiNode(
    std::unordered_map<const BasicBlock*, std::shared_ptr<ir::Value>> values)
    : Value{values.begin()->second->type()}, values_{std::move(values)} {
    assert(!values.empty());

    Type* firstTy = values_.begin()->second->type();
    assert(std::ranges::all_of(
        values_,
        [firstTy](Type* ty) { return ty == firstTy; },
        [](const auto& v) { return v.second->type(); }));
}

std::optional<std::shared_ptr<ir::Value>> PhiNode::valueFrom(
    const BasicBlock* bb) const {
    auto it = values_.find(bb);
    if (it == values_.end()) {
        return std::nullopt;
    }

    return it->second;
}
}  // namespace ir
