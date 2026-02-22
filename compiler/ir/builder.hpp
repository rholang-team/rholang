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

    Function* startFunction(std::string name, FunctionType* type);
    void finishFunction();

    BasicBlock* startBB();
    void finishBB();

    std::shared_ptr<IntImm> createIntImm(int value);
    std::shared_ptr<BoolImm> createBoolImm(bool value);

    std::shared_ptr<AllocaInstr> createAllocaInstr(Type* itemType);
    std::shared_ptr<CallInstr> createCallInstr(
        Function* callee,
        std::vector<std::shared_ptr<Value>> args);
    std::shared_ptr<NotInstr> createNotInstr(std::shared_ptr<Value> target);
    std::shared_ptr<NegInstr> createNegInstr(std::shared_ptr<Value> target);
    std::shared_ptr<LoadInstr> createLoadInstr(std::shared_ptr<Value> src);
    std::shared_ptr<StoreInstr> createStoreInstr(std::shared_ptr<Value> dest,
                                                 std::shared_ptr<Value> src);
    std::shared_ptr<AddInstr> createAddInstr(std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<SubInstr> createSubInstr(std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<MulInstr> createMulInstr(std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<CmpInstr> createCmpInstr(CmpInstr::Cond cond,
                                             std::shared_ptr<Value> lhs,
                                             std::shared_ptr<Value> rhs);
    std::shared_ptr<GetFieldPtrInstr> createGetFieldInstr(
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
