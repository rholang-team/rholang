#pragma once

#include <memory>

#include "compiler/ir/context.hpp"
#include "compiler/ir/instr.hpp"
#include "compiler/ir/module.hpp"
#include "compiler/ir/type.hpp"
#include "compiler/ir/value.hpp"

namespace ir {
class Builder {
    Context& ctx_;

    Module module_;
    std::unique_ptr<Function> function_{nullptr};
    std::unique_ptr<BasicBlock> bb_{nullptr};

public:
    explicit Builder(Context& ctx) : ctx_{ctx} {}

    Module build();

    VoidType* voidTy();
    BoolType* boolTy();
    IntType* intTy();
    PointerType* pointerTy();

    FunctionType* functionTy(Type* rettype, std::span<Type*> params);
    StructType* structTy(std::span<Type*> fields);

    FunctionSignature* addFunctionSignature(std::string name,
                                            FunctionType* type);

    Function* startFunction(std::string name, FunctionType* type);
    Function* startFunction(FunctionSignature* signature);

    void finishFunction();

    std::optional<FunctionSignature*> lookupSignature(const std::string& name);

    BasicBlock* startBb();
    BasicBlock* startBbAndLink();
    BasicBlock* finishBb();

    Function* curFunction();
    BasicBlock* curBb();

    std::shared_ptr<Instr> addToCurBb(std::shared_ptr<Instr> i);

    std::shared_ptr<IntImm> intImm(int value);
    std::shared_ptr<BoolImm> boolImm(bool value);
    std::shared_ptr<FnArgRef> fnArgRef(unsigned idx);
    std::shared_ptr<FnArgRef> fnArgRef(const FunctionSignature* fn,
                                       unsigned idx);
    std::shared_ptr<NullPtr> nullPtr();

    std::shared_ptr<AllocaInstr> allocaInstr(Type* itemType);
    std::shared_ptr<NewInstr> newInstr(Type* itemType);
    std::shared_ptr<CallInstr> callInstr(
        FunctionSignature* callee,
        std::vector<std::shared_ptr<Value>> args);
    std::shared_ptr<NotInstr> notInstr(std::shared_ptr<Value> target);
    std::shared_ptr<NegInstr> negInstr(std::shared_ptr<Value> target);
    std::shared_ptr<LoadInstr> loadInstr(Type* ty, std::shared_ptr<Value> src);
    std::shared_ptr<StoreInstr> storeInstr(Type* ty,
                                           std::shared_ptr<Value> dest,
                                           std::shared_ptr<Value> src);
    std::shared_ptr<AddInstr> addInstr(std::shared_ptr<Value> lhs,
                                       std::shared_ptr<Value> rhs);
    std::shared_ptr<SubInstr> subInstr(std::shared_ptr<Value> lhs,
                                       std::shared_ptr<Value> rhs);
    std::shared_ptr<MulInstr> mulInstr(std::shared_ptr<Value> lhs,
                                       std::shared_ptr<Value> rhs);
    std::shared_ptr<DivInstr> divInstr(std::shared_ptr<Value> lhs,
                                       std::shared_ptr<Value> rhs);
    std::shared_ptr<CmpInstr> cmpInstr(CmpInstr::Cond cond,
                                       std::shared_ptr<Value> lhs,
                                       std::shared_ptr<Value> rhs);
    std::shared_ptr<GetFieldPtrInstr> getFieldPtrInstr(
        StructType* structType,
        std::shared_ptr<Value> target,
        unsigned fieldIdx);
    std::shared_ptr<GotoInstr> gotoInstr(BasicBlock* dest);
    std::shared_ptr<BrInstr> brInstr(std::shared_ptr<Value> cond,
                                     BasicBlock* onTrue,
                                     BasicBlock* onFalse);
    std::shared_ptr<RetInstr> retInstr(
        std::optional<std::shared_ptr<Value>> value = std::nullopt);
};
}  // namespace ir
