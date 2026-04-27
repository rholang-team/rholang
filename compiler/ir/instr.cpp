#include "compiler/ir/instr.hpp"

namespace ir {
std::shared_ptr<AllocaInstr> AllocaInstr::create(Context& ctx, Type* itemType) {
    return std::shared_ptr<AllocaInstr>{
        new AllocaInstr{ctx.getPointerTy(), itemType}};
}

std::shared_ptr<NewInstr> NewInstr::create(Context& ctx, Type* itemType) {
    return std::shared_ptr<NewInstr>{
        new NewInstr{ctx.getPointerTy(), itemType}};
}

std::shared_ptr<CallInstr> CallInstr::create(
    const FunctionSignature* callee,
    std::vector<std::shared_ptr<Value>> args) {
    assert(callee);
    return std::shared_ptr<CallInstr>{new CallInstr{callee, std::move(args)}};
}

std::shared_ptr<NotInstr> NotInstr::create(std::shared_ptr<Value> target) {
    return std::shared_ptr<NotInstr>{new NotInstr{target}};
}

std::shared_ptr<NegInstr> NegInstr::create(std::shared_ptr<Value> target) {
    return std::shared_ptr<NegInstr>{new NegInstr{target}};
}

std::shared_ptr<LoadInstr> LoadInstr::create(Type* ty,
                                             std::shared_ptr<Value> src) {
    return std::shared_ptr<LoadInstr>{new LoadInstr{ty, src}};
}

std::shared_ptr<StoreInstr> StoreInstr::create(Context& ctx,
                                               Type* ty,
                                               std::shared_ptr<Value> dest,
                                               std::shared_ptr<Value> src) {
    return std::shared_ptr<StoreInstr>{
        new StoreInstr{ctx.getVoidTy(), ty, dest, src}};
}

std::shared_ptr<CmpInstr> CmpInstr::create(Context& ctx,
                                           Cond cond,
                                           std::shared_ptr<Value> lhs,
                                           std::shared_ptr<Value> rhs) {
    return std::shared_ptr<CmpInstr>{
        new CmpInstr{ctx.getBoolTy(), cond, lhs, rhs}};
}

std::shared_ptr<GetFieldPtrInstr> GetFieldPtrInstr::create(
    Context& ctx,
    StructType* structType,
    std::shared_ptr<Value> target,
    unsigned idx) {
    PointerType* resTy = ctx.getPointerTy();

    return std::shared_ptr<GetFieldPtrInstr>{
        new GetFieldPtrInstr{resTy, structType, target, idx}};
}

std::shared_ptr<GotoInstr> GotoInstr::create(Context& ctx,
                                             BasicBlock* dest,
                                             bool containsBackedge) {
    return std::shared_ptr<GotoInstr>{
        new GotoInstr{ctx.getVoidTy(), dest, containsBackedge}};
}

std::shared_ptr<BrInstr> BrInstr::create(Context& ctx,
                                         std::shared_ptr<Value> cond,
                                         BasicBlock* onTrue,
                                         BasicBlock* onFalse,
                                         bool containsBackedge) {
    return std::shared_ptr<BrInstr>{
        new BrInstr{ctx.getVoidTy(), cond, onTrue, onFalse, containsBackedge}};
}

std::shared_ptr<RetInstr> RetInstr::create(
    Context& ctx,
    std::optional<std::shared_ptr<Value>> value) {
    return std::shared_ptr<RetInstr>{new RetInstr{ctx.getVoidTy(), value}};
}

bool AllocaInstr::operator==(const Value& that) const {
    return utils::isa<AllocaInstr>(that) &&
           static_cast<const AllocaInstr&>(that).itemType_ == itemType_;
}
bool NewInstr::operator==(const Value& that) const {
    return utils::isa<NewInstr>(that) &&
           static_cast<const NewInstr&>(that).itemType_ == itemType_;
}
bool CallInstr::operator==(const Value& that) const {
    if (!utils::isa<CallInstr>(that)) {
        return false;
    }

    const auto& call = static_cast<const CallInstr&>(that);
    return *call.callee_ == *callee_ &&
           std::ranges::equal(
               call.args_,
               args_,
               [](const auto& a, const auto& b) { return *a == *b; });
}
bool NotInstr::operator==(const Value& that) const {
    return utils::isa<NotInstr>(that) &&
           *static_cast<const NotInstr&>(that).target_ == *target_;
}
bool NegInstr::operator==(const Value& that) const {
    return utils::isa<NegInstr>(that) &&
           *static_cast<const NegInstr&>(that).target_ == *target_;
}
bool LoadInstr::operator==(const Value& that) const {
    return utils::isa<LoadInstr>(that) && that.type() == type() &&
           *static_cast<const LoadInstr&>(that).src_ == *src_;
}
bool StoreInstr::operator==(const Value& that) const {
    return utils::isa<StoreInstr>(that) &&
           static_cast<const StoreInstr&>(that).storedValueType_ ==
               storedValueType_ &&
           *static_cast<const StoreInstr&>(that).dest_ == *dest_ &&
           *static_cast<const StoreInstr&>(that).src_ == *src_;
}
bool AddInstr::operator==(const Value& that) const {
    return utils::isa<AddInstr>(that) &&
           *static_cast<const AddInstr&>(that).lhs_ == *lhs_ &&
           *static_cast<const AddInstr&>(that).rhs_ == *rhs_;
}
bool SubInstr::operator==(const Value& that) const {
    return utils::isa<SubInstr>(that) &&
           *static_cast<const SubInstr&>(that).lhs_ == *lhs_ &&
           *static_cast<const SubInstr&>(that).rhs_ == *rhs_;
}
bool MulInstr::operator==(const Value& that) const {
    return utils::isa<MulInstr>(that) &&
           *static_cast<const MulInstr&>(that).lhs_ == *lhs_ &&
           *static_cast<const MulInstr&>(that).rhs_ == *rhs_;
}
bool DivInstr::operator==(const Value& that) const {
    return utils::isa<DivInstr>(that) &&
           *static_cast<const DivInstr&>(that).lhs_ == *lhs_ &&
           *static_cast<const DivInstr&>(that).rhs_ == *rhs_;
}
bool CmpInstr::operator==(const Value& that) const {
    return utils::isa<CmpInstr>(that) &&
           *static_cast<const CmpInstr&>(that).lhs_ == *lhs_ &&
           *static_cast<const CmpInstr&>(that).rhs_ == *rhs_;
}
bool GetFieldPtrInstr::operator==(const Value& that) const {
    if (!utils::isa<GetFieldPtrInstr>(that)) {
        return false;
    }

    const auto& gfp = static_cast<const GetFieldPtrInstr&>(that);

    return gfp.fieldIdx_ == fieldIdx_ && gfp.structType_ == structType_ &&
           *gfp.target_ == *target_;
}
bool GotoInstr::operator==(const Value& that) const {
    return utils::isa<GotoInstr>(that) &&
           *static_cast<const GotoInstr&>(that).dest_ == *dest_;
}
bool BrInstr::operator==(const Value& that) const {
    return utils::isa<BrInstr>(that) &&
           *static_cast<const BrInstr&>(that).cond_ == *cond_ &&
           *static_cast<const BrInstr&>(that).onTrue_ == *onTrue_ &&
           *static_cast<const BrInstr&>(that).onFalse_ == *onFalse_;
}
bool RetInstr::operator==(const Value& that) const {
    if (!utils::isa<RetInstr>(that)) {
        return false;
    }

    const auto& thatValue = static_cast<const RetInstr&>(that).value_;
    if (!thatValue.has_value()) {
        return !value_.has_value();
    }

    return value_.has_value() && **value_ == **thatValue;
}
}  // namespace ir
