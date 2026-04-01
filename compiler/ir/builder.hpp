#pragma once

#include <memory>

#include "compiler/ir/context.hpp"
#include "compiler/ir/instr.hpp"
#include "compiler/ir/module.hpp"
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

    VoidType* getVoidTy();
    BoolType* getBoolTy();
    IntType* getIntTy();
    PointerType* getPointerTy();

    FunctionType* getFunctionTy(Type* rettype, std::span<Type*> params);
    StructType* getStructTy(std::span<Type*> fields);

    void addFunctionSignature(std::unique_ptr<FunctionSignature> signature);

    Function* startFunction(FunctionSignature* signature);
    void finishFunction();

    std::optional<FunctionSignature*> lookupSignature(const std::string& name);

    BasicBlock* startBB();
    BasicBlock* startBbAndLink();
    BasicBlock* finishBB();

    Function* curFunction();
    BasicBlock* curBasicBlock();

    std::shared_ptr<Instr> addToCurBB(std::shared_ptr<Instr> i);

    std::shared_ptr<IntImm> createIntImm(int value);
    std::shared_ptr<BoolImm> createBoolImm(bool value);
    std::shared_ptr<FnArgRef> createFnArgRef(const FunctionSignature* fn,
                                             unsigned idx);
    std::shared_ptr<NullPtr> createNullPtr();

    std::shared_ptr<AllocaInstr> createAllocaInstr(Type* itemType);
    std::shared_ptr<NewInstr> createNewInstr(Type* itemType);
    std::shared_ptr<CallInstr> createCallInstr(
        FunctionSignature* callee,
        std::vector<std::shared_ptr<Value>> args);
    std::shared_ptr<NotInstr> createNotInstr(std::shared_ptr<Value> target);
    std::shared_ptr<NegInstr> createNegInstr(std::shared_ptr<Value> target);
    std::shared_ptr<LoadInstr> createLoadInstr(Type* ty,
                                               std::shared_ptr<Value> src);
    std::shared_ptr<StoreInstr> createStoreInstr(Type* ty,
                                                 std::shared_ptr<Value> dest,
                                                 std::shared_ptr<Value> src);
    std::shared_ptr<AddInstr> createAddInstr(std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<SubInstr> createSubInstr(std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<MulInstr> createMulInstr(std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<DivInstr> createDivInstr(std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<CmpInstr> createCmpInstr(CmpInstr::Cond cond,
                                             std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<GetFieldPtrInstr> createGetFieldPtrInstr(
        StructType* structType,
        std::shared_ptr<Value> target,
        unsigned fieldIdx);
    std::shared_ptr<GotoInstr> createGotoInstr(BasicBlock* dest);
    std::shared_ptr<BrInstr> createBrInstr(std::shared_ptr<Value> cond,
                                           BasicBlock* onTrue,
                                           BasicBlock* onFalse);
    std::shared_ptr<RetInstr> createRetInstr(
        std::optional<std::shared_ptr<Value>> value = std::nullopt);
};
}  // namespace ir
