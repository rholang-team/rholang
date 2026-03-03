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

std::shared_ptr<GotoInstr> GotoInstr::create(Context& ctx, BasicBlock* dest) {
    return std::shared_ptr<GotoInstr>{new GotoInstr{ctx.getVoidTy(), dest}};
}

std::shared_ptr<BrInstr> BrInstr::create(Context& ctx,
                                         std::shared_ptr<Value> cond,
                                         BasicBlock* onTrue,
                                         BasicBlock* onFalse) {
    return std::shared_ptr<BrInstr>{
        new BrInstr{ctx.getVoidTy(), cond, onTrue, onFalse}};
}

std::shared_ptr<RetInstr> RetInstr::create(
    Context& ctx,
    std::optional<std::shared_ptr<Value>> value) {
    return std::shared_ptr<RetInstr>{new RetInstr{ctx.getVoidTy(), value}};
}
}  // namespace ir
