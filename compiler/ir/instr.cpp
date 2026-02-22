#include "compiler/ir/instr.hpp"

#include "utils/match.hpp"

namespace ir {
Instr::Instr(Type* type) : Value{type} {}

bool Instr::isTerminator() const {
    return false;
}

InstrWithResult::InstrWithResult(std::shared_ptr<TmpVar> dest)
    : Instr{dest->type()}, dest_{dest} {}

std::shared_ptr<TmpVar> InstrWithResult::dest() const {
    return dest_;
}

NotInstr::NotInstr(std::shared_ptr<TmpVar> dest, std::shared_ptr<Value> target)
    : InstrWithResult{dest}, target_{target} {
    assert(utils::isa<BoolType>(target->type()));
    assert(utils::isa<BoolType>(dest->type()));
}

std::shared_ptr<NotInstr> NotInstr::get(Context& ctx,
                                        std::shared_ptr<Value> target) {
    return std::shared_ptr<NotInstr>{
        new NotInstr{TmpVar::get(ctx, ctx.getBoolTy()), target}};
}

NegInstr::NegInstr(std::shared_ptr<TmpVar> dest, std::shared_ptr<Value> target)
    : InstrWithResult{dest}, target_{target} {
    assert(utils::isa<IntType>(target->type()));
}

std::shared_ptr<NegInstr> NegInstr::get(Context& ctx,
                                        std::shared_ptr<Value> target) {
    return std::shared_ptr<NegInstr>{
        new NegInstr{TmpVar::get(ctx, ctx.getIntTy()), target}};
}

std::shared_ptr<Value> NegInstr::target() const {
    return target_;
}

LoadInstr::LoadInstr(std::shared_ptr<TmpVar> dest, std::shared_ptr<Value> src)
    : InstrWithResult{dest}, src_{src} {
    assert(dest->type() == src->type());
}

std::shared_ptr<LoadInstr> LoadInstr::get(Context& ctx,
                                          std::shared_ptr<Value> src) {
    PointerType* resTy = dynamic_cast<PointerType*>(src->type());

    return std::shared_ptr<LoadInstr>{
        new LoadInstr{TmpVar::get(ctx, resTy->underlying()), src}};
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

std::shared_ptr<StoreInstr> StoreInstr::get(Context& ctx,
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

CmpInstr::CmpInstr(std::shared_ptr<TmpVar> dest,
                   Cond cond,
                   std::shared_ptr<Value> lhs,
                   std::shared_ptr<Value> rhs)
    : InstrWithResult{dest}, cond_{cond}, lhs_{lhs}, rhs_{rhs} {
    assert(lhs->type() == rhs->type());
}

std::shared_ptr<CmpInstr> CmpInstr::get(Context& ctx,
                                        Cond cond,
                                        std::shared_ptr<Value> lhs,
                                        std::shared_ptr<Value> rhs) {
    return std::shared_ptr<CmpInstr>{
        new CmpInstr{TmpVar::get(ctx, ctx.getBoolTy()), cond, lhs, rhs}};
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

GetFieldPtr::GetFieldPtr(std::shared_ptr<TmpVar> dest,
                         std::shared_ptr<Value> target,
                         unsigned fieldIdx)
    : InstrWithResult{dest}, target_{target}, fieldIdx_{fieldIdx} {}

std::shared_ptr<GetFieldPtr> GetFieldPtr::get(Context& ctx,
                                              std::shared_ptr<Value> target,
                                              unsigned fieldIdx) {
    Type* resTy = dynamic_cast<StructType*>(target->type())->fields()[fieldIdx];

    return std::shared_ptr<GetFieldPtr>{
        new GetFieldPtr{TmpVar::get(ctx, resTy), target, fieldIdx}};
}

GotoInstr::GotoInstr(VoidType* ty, BasicBlock* dest) : Instr{ty}, dest_{dest} {}

std::shared_ptr<GotoInstr> GotoInstr::get(Context& ctx, BasicBlock* dest) {
    return std::shared_ptr<GotoInstr>{new GotoInstr{ctx.getVoidTy(), dest}};
}

bool GotoInstr::isTerminator() const {
    return true;
}

BrInstr::BrInstr(VoidType* ty,
                 std::shared_ptr<Value> cond,
                 BasicBlock* onTrue,
                 BasicBlock* onFalse)
    : Instr{ty}, cond_{cond}, onTrue_{onTrue}, onFalse_{onFalse} {}

std::shared_ptr<BrInstr> BrInstr::get(Context& ctx,
                                      std::shared_ptr<Value> cond,
                                      BasicBlock* onTrue,
                                      BasicBlock* onFalse) {
    return std::shared_ptr<BrInstr>{
        new BrInstr{ctx.getVoidTy(), cond, onTrue, onFalse}};
}

bool BrInstr::isTerminator() const {
    return true;
}

RetInstr::RetInstr(VoidType* ty, std::optional<std::shared_ptr<Value>> value)
    : Instr{ty}, value_{value} {}

std::shared_ptr<RetInstr> RetInstr::get(
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
