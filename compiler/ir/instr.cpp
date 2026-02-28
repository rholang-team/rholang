#include "compiler/ir/instr.hpp"

#include "utils/match.hpp"

namespace ir {
Instr::Instr(Type* type) : Value{type} {}

bool Instr::isTerminator() const {
    return false;
}

AllocaInstr::AllocaInstr(Type* ty) : Instr{ty} {}

std::shared_ptr<AllocaInstr> AllocaInstr::create(Context& ctx, Type* itemType) {
    return std::shared_ptr<AllocaInstr>{
        new AllocaInstr{PointerType::get(ctx, itemType)}};
}

Type* AllocaInstr::itemType() const {
    return dynamic_cast<PointerType*>(type())->underlying();
}

CallInstr::CallInstr(std::shared_ptr<FunctionSignature> callee,
                     std::vector<std::shared_ptr<Value>> args)
    : Instr{dynamic_cast<const FunctionType*>(callee->type())->rettype()},
      args_{std::move(args)} {}

std::shared_ptr<CallInstr> CallInstr::create(
    std::shared_ptr<FunctionSignature> callee,
    std::vector<std::shared_ptr<Value>> args) {
    return std::shared_ptr<CallInstr>{new CallInstr{callee, std::move(args)}};
}

std::shared_ptr<FunctionSignature> CallInstr::callee() const {
    return callee_;
}

std::vector<std::shared_ptr<Value>>& CallInstr::args() {
    return args_;
}

const std::vector<std::shared_ptr<Value>>& CallInstr::args() const {
    return args_;
}

NotInstr::NotInstr(std::shared_ptr<Value> target)
    : Instr{target->type()}, target_{target} {
    assert(utils::isa<BoolType>(target->type()));
}

std::shared_ptr<NotInstr> NotInstr::create(std::shared_ptr<Value> target) {
    return std::shared_ptr<NotInstr>{new NotInstr{target}};
}

NegInstr::NegInstr(std::shared_ptr<Value> target)
    : Instr{target->type()}, target_{target} {
    assert(utils::isa<IntType>(target->type()));
}

std::shared_ptr<NegInstr> NegInstr::create(std::shared_ptr<Value> target) {
    return std::shared_ptr<NegInstr>{new NegInstr{target}};
}

std::shared_ptr<Value> NegInstr::target() const {
    return target_;
}

LoadInstr::LoadInstr(Type* ty, std::shared_ptr<Value> src)
    : Instr{ty}, src_{src} {}

std::shared_ptr<LoadInstr> LoadInstr::create(std::shared_ptr<Value> src) {
    PointerType* resTy = dynamic_cast<PointerType*>(src->type());

    return std::shared_ptr<LoadInstr>{new LoadInstr{resTy, src}};
}

std::shared_ptr<Value> LoadInstr::src() const {
    return src_;
}

StoreInstr::StoreInstr(VoidType* ty,
                       std::shared_ptr<Value> dest,
                       std::shared_ptr<Value> src)
    : Instr{ty}, dest_{dest}, src_{src} {
    assert(dynamic_cast<PointerType*>(dest->type())->underlying() ==
           src->type());
}

std::shared_ptr<StoreInstr> StoreInstr::create(Context& ctx,
                                               std::shared_ptr<Value> dest,
                                               std::shared_ptr<Value> src) {
    return std::shared_ptr<StoreInstr>{
        new StoreInstr{ctx.getVoidTy(), dest, src}};
}

std::shared_ptr<Value> StoreInstr::dest() const {
    return dest_;
}
std::shared_ptr<Value> StoreInstr::src() const {
    return src_;
}

CmpInstr::CmpInstr(BoolType* ty,
                   Cond cond,
                   std::shared_ptr<Value> lhs,
                   std::shared_ptr<Value> rhs)
    : Instr{ty}, cond_{cond}, lhs_{lhs}, rhs_{rhs} {
    assert(lhs->type() == rhs->type());
}

std::shared_ptr<CmpInstr> CmpInstr::create(Context& ctx,
                                           Cond cond,
                                           std::shared_ptr<Value> lhs,
                                           std::shared_ptr<Value> rhs) {
    return std::shared_ptr<CmpInstr>{
        new CmpInstr{ctx.getBoolTy(), cond, lhs, rhs}};
}

CmpInstr::Cond CmpInstr::cond() const {
    return cond_;
}

std::shared_ptr<Value> CmpInstr::lhs() const {
    return lhs_;
}

std::shared_ptr<Value> CmpInstr::rhs() const {
    return rhs_;
}

GetFieldPtrInstr::GetFieldPtrInstr(Type* ty,
                                   std::shared_ptr<Value> target,
                                   unsigned idx)
    : Instr{ty}, target_{target}, fieldIdx_{idx} {}

std::shared_ptr<GetFieldPtrInstr> GetFieldPtrInstr::create(
    std::shared_ptr<Value> target,
    unsigned idx) {
    Type* resTy = dynamic_cast<StructType*>(target->type())->fields()[idx];

    return std::shared_ptr<GetFieldPtrInstr>{
        new GetFieldPtrInstr{resTy, target, idx}};
}

std::shared_ptr<Value> GetFieldPtrInstr::target() const {
    return target_;
}

unsigned GetFieldPtrInstr::fieldIdx() const {
    return fieldIdx_;
}

GotoInstr::GotoInstr(VoidType* ty, BasicBlock* dest) : Instr{ty}, dest_{dest} {}

std::shared_ptr<GotoInstr> GotoInstr::create(Context& ctx, BasicBlock* dest) {
    return std::shared_ptr<GotoInstr>{new GotoInstr{ctx.getVoidTy(), dest}};
}

bool GotoInstr::isTerminator() const {
    return true;
}

BasicBlock* GotoInstr::dest() const {
    return dest_;
}

BrInstr::BrInstr(VoidType* ty,
                 std::shared_ptr<Value> cond,
                 BasicBlock* onTrue,
                 BasicBlock* onFalse)
    : Instr{ty}, cond_{cond}, onTrue_{onTrue}, onFalse_{onFalse} {}

std::shared_ptr<BrInstr> BrInstr::create(Context& ctx,
                                         std::shared_ptr<Value> cond,
                                         BasicBlock* onTrue,
                                         BasicBlock* onFalse) {
    return std::shared_ptr<BrInstr>{
        new BrInstr{ctx.getVoidTy(), cond, onTrue, onFalse}};
}

bool BrInstr::isTerminator() const {
    return true;
}

std::shared_ptr<Value> BrInstr::cond() const {
    return cond_;
}

BasicBlock* BrInstr::onTrue() const {
    return onTrue_;
}

BasicBlock* BrInstr::onFalse() const {
    return onFalse_;
}

RetInstr::RetInstr(VoidType* ty, std::optional<std::shared_ptr<Value>> value)
    : Instr{ty}, value_{value} {}

std::shared_ptr<RetInstr> RetInstr::create(
    Context& ctx,
    std::optional<std::shared_ptr<Value>> value) {
    return std::shared_ptr<RetInstr>{new RetInstr{ctx.getVoidTy(), value}};
}

std::optional<std::shared_ptr<Value>> RetInstr::value() const {
    return value_;
}

bool RetInstr::isTerminator() const {
    return true;
}
}  // namespace ir
