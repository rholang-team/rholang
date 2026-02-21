#pragma once

#include <cassert>
#include <memory>
#include <optional>

#include "compiler/ir/value.hpp"
#include "utils/match.hpp"

namespace ir {
class BasicBlock;

struct Instr : public Value {
    bool isTerminator;

    explicit Instr(Type* type, bool isTerminator = false)
        : Value{type}, isTerminator{isTerminator} {}
};

class NotInstr final : public Instr {
    std::shared_ptr<Value> target_;

public:
    NotInstr(Context& ctx, std::shared_ptr<Value> target)
        : Instr{ctx.getBoolTy()}, target_{target} {
        assert(utils::isa<BoolType>(target->type));
    }
};

class NegInstr final : public Instr {
    std::shared_ptr<Value> target_;

public:
    NegInstr(Context& ctx, std::shared_ptr<Value> target)
        : Instr{ctx.getIntTy()}, target_{target} {
        assert(utils::isa<IntType>(target->type));
    }
};

class AllocaInstr final : public Instr {
    Type* itemTy_;

public:
    AllocaInstr(Context& ctx, Type* itemTy, size_t alignment)
        : Instr{PointerType::get(ctx, itemTy)}, itemTy_{itemTy} {}
};

class LoadInstr final : public Instr {
    Type* itemTy_;
    std::shared_ptr<Value> src_;

public:
    LoadInstr(Context& ctx, Type* itemTy, std::shared_ptr<Value> src)
        : Instr{ctx.getVoidTy()}, itemTy_{itemTy}, src_{src} {}
};

class StoreInstr final : public Instr {
    std::shared_ptr<Value> dest_;
    std::shared_ptr<Value> src_;

public:
    StoreInstr(Context& ctx,
               std::shared_ptr<Value> dest,
               std::shared_ptr<Value> src)
        : Instr{ctx.getVoidTy()}, dest_{dest}, src_{src} {}
};

#define MAKE_BINARY_INSTR(name, ty)                             \
    class name final : public Instr {                           \
        std::shared_ptr<Value> lhs_, rhs_;                      \
                                                                \
    public:                                                     \
        name(Context& ctx,                                      \
             std::shared_ptr<Value> lhs,                        \
             std::shared_ptr<Value> rhs)                        \
            : Instr{ctx.get##ty##Ty()}, lhs_{lhs}, rhs_{rhs} {} \
    };

MAKE_BINARY_INSTR(AddInstr, Int);
MAKE_BINARY_INSTR(SubInstr, Int);
MAKE_BINARY_INSTR(MulInstr, Int);
MAKE_BINARY_INSTR(AndInstr, Bool);
MAKE_BINARY_INSTR(OrInstr, Bool);

#undef MAKE_BINARY_INSTR

class CmpInstr final : public Instr {
public:
    enum class Cond {
        Eq,
        Ne,
        Lt,
        Gt,
        Le,
        Ge,
    };

private:
    Cond cond_;
    std::shared_ptr<Value> lhs_, rhs_;

public:
    CmpInstr(Context& ctx,
             Cond cond,
             std::shared_ptr<Value> lhs,
             std::shared_ptr<Value> rhs)
        : Instr{ctx.getBoolTy()}, cond_{cond}, lhs_{lhs}, rhs_{rhs} {}
};

class GetElementPtr final : public Instr {
    std::vector<std::shared_ptr<Value>> indices_;

public:
    GetElementPtr(Type* type, std::vector<std::shared_ptr<Value>> indices)
        : Instr{type}, indices_{std::move(indices)} {}
};

class GotoInstr final : public Instr {
    BasicBlock* dest_;

public:
    GotoInstr(Context& ctx, BasicBlock* to)
        : Instr{ctx.getVoidTy(), true}, dest_{to} {}
};

class BrInstr final : public Instr {
    std::shared_ptr<Value> cond_;
    BasicBlock* onTrue_;
    BasicBlock* onFalse_;

public:
    BrInstr(Context& ctx,
            std::shared_ptr<Value> cond,
            BasicBlock* onTrue,
            BasicBlock* onFalse)
        : Instr{ctx.getVoidTy(), true},
          cond_{cond},
          onTrue_{onTrue},
          onFalse_{onFalse} {}
};

class RetInstr final : public Instr {
    std::optional<std::shared_ptr<Value>> value_;

public:
    explicit RetInstr(
        Context& ctx,
        std::optional<std::shared_ptr<Value>> value = std::nullopt)
        : Instr{ctx.getVoidTy(), true}, value_{value} {}

    std::shared_ptr<Value> value() const;
};
}  // namespace ir