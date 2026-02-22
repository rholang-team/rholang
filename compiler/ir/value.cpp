#include "compiler/ir/value.hpp"

namespace ir {
Type* Value::type() const {
    return type_;
}

std::shared_ptr<IntImm> IntImm::create(Context& ctx, int value) {
    return std::shared_ptr<IntImm>{new IntImm{ctx.getIntTy(), value}};
}

std::shared_ptr<BoolImm> BoolImm::create(Context& ctx, bool value) {
    return std::shared_ptr<BoolImm>{new BoolImm{ctx.getBoolTy(), value}};
}
}  // namespace ir
