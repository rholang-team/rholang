#include "compiler/ir/builder.hpp"

#include <utility>

namespace ir {
Module Builder::build() {
    return std::move(module_);
}
VoidType* Builder::voidTy() {
    return ctx_.getVoidTy();
}
BoolType* Builder::boolTy() {
    return ctx_.getBoolTy();
}
IntType* Builder::intTy() {
    return ctx_.getIntTy();
}
PointerType* Builder::pointerTy() {
    return ctx_.getPointerTy();
}

FunctionType* Builder::functionTy(Type* rettype, std::span<Type*> params) {
    return FunctionType::get(ctx_, rettype, params);
}

StructType* Builder::structTy(std::span<Type*> fields) {
    return StructType::get(ctx_, fields);
}

std::shared_ptr<GlobalPtr> Builder::addGlobal(std::string name, Type* ty) {
    return module_.addGlobal(name, GlobalPtr::create(ctx_, name, ty));
}

FunctionSignature* Builder::addFunctionSignature(std::string name,
                                                 FunctionType* type) {
    return module_.addSignature(
        std::make_unique<FunctionSignature>(std::move(name), type));
}

Function* Builder::startFunction(std::string name, FunctionType* type) {
    return startFunction(addFunctionSignature(std::move(name), type));
}

Function* Builder::startFunction(FunctionSignature* signature) {
    assert(!function_);

    function_ = std::make_unique<Function>(signature);
    return function_.get();
}

void Builder::finishFunction() {
    assert(function_);

    module_.addFunction(
        std::exchange(function_, std::unique_ptr<Function>{nullptr}));
}

std::optional<FunctionSignature*> Builder::lookupSignature(
    const std::string& name) {
    auto it = module_.signatures().find(name);

    if (it == module_.signatures().end()) {
        return std::nullopt;
    }
    return it->second.get();
}

BasicBlock* Builder::startBb() {
    assert(!bb_);

    bb_ = std::make_unique<BasicBlock>();
    return bb_.get();
}

BasicBlock* Builder::startBbAndLink() {
    if (!bb_) {
        return startBb();
    }

    BasicBlock* prev = bb_.get();
    finishBb();
    BasicBlock* res = startBb();
    prev->addInstr(gotoInstr(res));
    return res;
}

BasicBlock* Builder::finishBb() {
    assert(bb_);
    assert(function_);

    BasicBlock* res = bb_.get();

    function_->addBB(std::exchange(bb_, std::unique_ptr<BasicBlock>{nullptr}));

    return res;
}

Function* Builder::curFunction() {
    return function_.get();
}

BasicBlock* Builder::curBb() {
    return bb_.get();
}

std::shared_ptr<Instr> Builder::addToCurBb(std::shared_ptr<Instr> i) {
    assert(bb_);
    bb_->addInstr(i);
    return i;
}

std::shared_ptr<IntImm> Builder::intImm(int value) {
    return IntImm::create(ctx_, value);
}

std::shared_ptr<BoolImm> Builder::boolImm(bool value) {
    return BoolImm::create(ctx_, value);
}

std::shared_ptr<FnArgRef> Builder::fnArgRef(unsigned idx) {
    assert(function_);
    return FnArgRef::create(function_->signature(), idx);
}

std::shared_ptr<FnArgRef> Builder::fnArgRef(const FunctionSignature* fn,
                                            unsigned idx) {
    return FnArgRef::create(fn, idx);
}

std::shared_ptr<NullPtr> Builder::nullPtr() {
    return NullPtr::create(ctx_);
}

std::shared_ptr<AllocaInstr> Builder::allocaInstr(Type* itemType) {
    return AllocaInstr::create(ctx_, itemType);
}

std::shared_ptr<NewInstr> Builder::newInstr(Type* itemType) {
    return NewInstr::create(ctx_, itemType);
}

std::shared_ptr<CallInstr> Builder::callInstr(
    FunctionSignature* callee,
    std::vector<std::shared_ptr<Value>> args) {
    return CallInstr::create(callee, std::move(args));
}

std::shared_ptr<NotInstr> Builder::notInstr(std::shared_ptr<Value> target) {
    return NotInstr::create(target);
}

std::shared_ptr<NegInstr> Builder::negInstr(std::shared_ptr<Value> target) {
    return NegInstr::create(target);
}

std::shared_ptr<LoadInstr> Builder::loadInstr(Type* ty,
                                              std::shared_ptr<Value> src) {
    return LoadInstr::create(ty, src);
}

std::shared_ptr<StoreInstr> Builder::storeInstr(Type* ty,
                                                std::shared_ptr<Value> dest,
                                                std::shared_ptr<Value> src) {
    return StoreInstr::create(ctx_, ty, dest, src);
}

std::shared_ptr<AddInstr> Builder::addInstr(std::shared_ptr<Value> lhs,
                                            std::shared_ptr<Value> rhs) {
    return AddInstr::create(lhs, rhs);
}

std::shared_ptr<SubInstr> Builder::subInstr(std::shared_ptr<Value> lhs,
                                            std::shared_ptr<Value> rhs) {
    return SubInstr::create(lhs, rhs);
}

std::shared_ptr<MulInstr> Builder::mulInstr(std::shared_ptr<Value> lhs,
                                            std::shared_ptr<Value> rhs) {
    return MulInstr::create(lhs, rhs);
}

std::shared_ptr<DivInstr> Builder::divInstr(std::shared_ptr<Value> lhs,
                                            std::shared_ptr<Value> rhs) {
    return DivInstr::create(lhs, rhs);
}

std::shared_ptr<CmpInstr> Builder::cmpInstr(CmpInstr::Cond cond,
                                            std::shared_ptr<Value> lhs,
                                            std::shared_ptr<Value> rhs) {
    return CmpInstr::create(ctx_, cond, lhs, rhs);
}

std::shared_ptr<GetFieldPtrInstr> Builder::getFieldPtrInstr(
    StructType* structType,
    std::shared_ptr<Value> target,
    unsigned idx) {
    return GetFieldPtrInstr::create(ctx_, structType, target, idx);
}

std::shared_ptr<GotoInstr> Builder::gotoInstr(BasicBlock* dest) {
    return GotoInstr::create(ctx_, dest);
}

std::shared_ptr<BrInstr> Builder::brInstr(std::shared_ptr<Value> cond,
                                          BasicBlock* onTrue,
                                          BasicBlock* onFalse) {
    return BrInstr::create(ctx_, cond, onTrue, onFalse);
}

std::shared_ptr<RetInstr> Builder::retInstr(
    std::optional<std::shared_ptr<Value>> value) {
    return RetInstr::create(ctx_, value);
}
}  // namespace ir