#include "compiler/ir/builder.hpp"

#include <utility>

namespace ir {
Module Builder::build() {
    return std::move(module_);
}

Function* Builder::startFunction(std::string name, FunctionType* type) {
    assert(!function_);

    function_ = std::make_unique<Function>(std::move(name), type);
    return function_.get();
}

void Builder::finishFunction() {
    assert(function_);

    module_.addFunction(
        std::exchange(function_, std::unique_ptr<Function>{nullptr}));
}

BasicBlock* Builder::startBB() {
    assert(!bb_);

    bb_ = std::make_unique<BasicBlock>();
    return bb_.get();
}

void Builder::finishBB() {
    assert(bb_);
    assert(function_);

    function_->bbs().emplace_back(
        std::exchange(bb_, std::unique_ptr<BasicBlock>{nullptr}));
}

std::shared_ptr<IntImm> Builder::createIntImm(int value) {
    return IntImm::create(ctx_, value);
}

std::shared_ptr<BoolImm> Builder::createBoolImm(bool value) {
    return BoolImm::create(ctx_, value);
}

std::shared_ptr<AllocaInstr> Builder::createAllocaInstr(Type* itemType) {
    return AllocaInstr::create(ctx_, itemType);
}

std::shared_ptr<CallInstr> Builder::createCallInstr(
    Function* callee,
    std::vector<std::shared_ptr<Value>> args) {
    return CallInstr::create(callee, std::move(args));
}

std::shared_ptr<NotInstr> Builder::createNotInstr(
    std::shared_ptr<Value> target) {
    return NotInstr::create(target);
}

std::shared_ptr<NegInstr> Builder::createNegInstr(
    std::shared_ptr<Value> target) {
    return NegInstr::create(target);
}

std::shared_ptr<LoadInstr> Builder::createLoadInstr(
    std::shared_ptr<Value> src) {
    return LoadInstr::create(src);
}

std::shared_ptr<StoreInstr> Builder::createStoreInstr(
    std::shared_ptr<Value> dest,
    std::shared_ptr<Value> src) {
    return StoreInstr::create(ctx_, dest, src);
}

std::shared_ptr<AddInstr> Builder::createAddInstr(std::shared_ptr<Value> lhs,
                                                  std::shared_ptr<Value> rhs) {
    return AddInstr::create(lhs, rhs);
}

std::shared_ptr<SubInstr> Builder::createSubInstr(std::shared_ptr<Value> lhs,
                                                  std::shared_ptr<Value> rhs) {
    return SubInstr::create(lhs, rhs);
}

std::shared_ptr<MulInstr> Builder::createMulInstr(std::shared_ptr<Value> lhs,
                                                  std::shared_ptr<Value> rhs) {
    return MulInstr::create(lhs, rhs);
}

std::shared_ptr<CmpInstr> Builder::createCmpInstr(CmpInstr::Cond cond,
                                                  std::shared_ptr<Value> lhs,
                                                  std::shared_ptr<Value> rhs) {
    return CmpInstr::create(ctx_, cond, lhs, rhs);
}

std::shared_ptr<GetFieldPtrInstr> Builder::createGetFieldInstr(
    std::shared_ptr<Value> target,
    unsigned fieldIdx) {
    return GetFieldPtrInstr::create(target, fieldIdx);
}

std::shared_ptr<GotoInstr> Builder::createGotoInstr(BasicBlock* dest) {
    return GotoInstr::create(ctx_, dest);
}

std::shared_ptr<BrInstr> Builder::createBrInstr(std::shared_ptr<Value> cond,
                                                BasicBlock* onTrue,
                                                BasicBlock* onFalse) {
    return BrInstr::create(ctx_, cond, onTrue, onFalse);
}

std::shared_ptr<RetInstr> Builder::createRetInstr(
    std::optional<std::shared_ptr<Value>> value) {
    return RetInstr::create(ctx_, value);
}
}  // namespace ir