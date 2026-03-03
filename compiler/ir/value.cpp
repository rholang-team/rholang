#include "compiler/ir/value.hpp"

namespace ir {
Type* Value::type() const {
    return type_;
}

std::shared_ptr<IntImm> IntImm::create(Context& ctx, int value) {
    return std::shared_ptr<IntImm>{new IntImm{ctx.getIntTy(), value}};
}

int IntImm::value() const {
    return value_;
}

std::shared_ptr<BoolImm> BoolImm::create(Context& ctx, bool value) {
    return std::shared_ptr<BoolImm>{new BoolImm{ctx.getBoolTy(), value}};
}

bool BoolImm::value() const {
    return value_;
}

std::shared_ptr<FnArgRef> FnArgRef::create(Function* fn, unsigned idx) {
    return std::shared_ptr<FnArgRef>{
        new FnArgRef{fn->signature()->type()->params()[idx], idx}};
}

unsigned FnArgRef::idx() const {
    return idx_;
}
}  // namespace ir
