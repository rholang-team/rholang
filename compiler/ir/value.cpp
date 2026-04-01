#include "compiler/ir/value.hpp"

#include "compiler/ir/function.hpp"

namespace ir {
std::shared_ptr<IntImm> IntImm::create(Context& ctx, int value) {
    return std::shared_ptr<IntImm>{new IntImm{ctx.getIntTy(), value}};
}

std::shared_ptr<BoolImm> BoolImm::create(Context& ctx, bool value) {
    return std::shared_ptr<BoolImm>{new BoolImm{ctx.getBoolTy(), value}};
}

std::shared_ptr<FnArgRef> FnArgRef::create(const FunctionSignature* fn,
                                           unsigned idx) {
    return std::shared_ptr<FnArgRef>{
        new FnArgRef{fn->type()->params()[idx], idx}};
}

std::shared_ptr<NullPtr> NullPtr::create(Context& ctx) {
    return std::shared_ptr<NullPtr>{new NullPtr{ctx.getPointerTy()}};
}
}  // namespace ir
