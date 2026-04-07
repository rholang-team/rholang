#include "compiler/ir/value.hpp"

#include "compiler/ir/function.hpp"
#include "utils/match.hpp"

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

std::shared_ptr<GlobalPtr> GlobalPtr::create(Context& ctx,
                                             std::string name,
                                             Type* valueTy) {
    return std::shared_ptr<GlobalPtr>{
        new GlobalPtr{ctx.getPointerTy(), std::move(name), valueTy}};
}

std::shared_ptr<NullPtr> NullPtr::create(Context& ctx) {
    return std::shared_ptr<NullPtr>{new NullPtr{ctx.getPointerTy()}};
}

bool IntImm::operator==(const Value& that) const {
    return utils::isa<IntImm>(&that) &&
           static_cast<const IntImm&>(that).value_ == value_;
}

bool BoolImm::operator==(const Value& that) const {
    return utils::isa<BoolImm>(&that) &&
           static_cast<const BoolImm&>(that).value_ == value_;
}

bool FnArgRef::operator==(const Value& that) const {
    return utils::isa<FnArgRef>(&that) &&
           static_cast<const FnArgRef&>(that).idx_ == idx_ &&
           that.type() == type();
}

bool GlobalPtr::operator==(const Value& that) const {
    return utils::isa<GlobalPtr>(&that) &&
           static_cast<const GlobalPtr&>(that).valueTy_ == valueTy_ &&
           static_cast<const GlobalPtr&>(that).name_ == name_;
}

bool NullPtr::operator==(const Value& that) const {
    return utils::isa<NullPtr>(&that) && that.type() == type();
}
}  // namespace ir
