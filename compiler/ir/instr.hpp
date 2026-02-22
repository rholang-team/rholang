#pragma once

#include <cassert>
#include <memory>
#include <optional>

#include "compiler/ir/value.hpp"

namespace ir {
class BasicBlock;

class Instr : public Value {
protected:
    explicit Instr(Type* type);

public:
    virtual bool isTerminator() const;
};

class InstrWithResult : public Instr {
    std::shared_ptr<TmpVar> dest_;

protected:
    explicit InstrWithResult(std::shared_ptr<TmpVar> dest);

public:
    std::shared_ptr<TmpVar> dest() const;
};

class NotInstr final : public InstrWithResult {
    std::shared_ptr<Value> target_;

    NotInstr(std::shared_ptr<TmpVar> dest, std::shared_ptr<Value> target);

public:
    static std::shared_ptr<NotInstr> get(Context& ctx,
                                         std::shared_ptr<Value> target);

    std::shared_ptr<Value> target() const;
};

class NegInstr final : public InstrWithResult {
    std::shared_ptr<Value> target_;

    NegInstr(std::shared_ptr<TmpVar> dest, std::shared_ptr<Value> target);

public:
    static std::shared_ptr<NegInstr> get(Context& ctx,
                                         std::shared_ptr<Value> target);

    std::shared_ptr<Value> target() const;
};

class LoadInstr final : public InstrWithResult {
    std::shared_ptr<Value> src_;

    LoadInstr(std::shared_ptr<TmpVar> dest, std::shared_ptr<Value> src);

public:
    static std::shared_ptr<LoadInstr> get(Context& ctx,
                                          std::shared_ptr<Value> src);

    std::shared_ptr<Value> src() const;
};

class StoreInstr final : public Instr {
    std::shared_ptr<Value> dest_;
    std::shared_ptr<Value> src_;

    StoreInstr(VoidType* ty,
               std::shared_ptr<Value> dest,
               std::shared_ptr<Value> src);

public:
    static std::shared_ptr<StoreInstr> get(Context& ctx,
                                           std::shared_ptr<Value> dest,
                                           std::shared_ptr<Value> src);

    std::shared_ptr<Value> dest() const;
    std::shared_ptr<Value> src() const;
};

#define MAKE_BINARY_INSTR(name, ty)                                    \
    class name final : public InstrWithResult {                        \
        std::shared_ptr<Value> lhs_, rhs_;                             \
                                                                       \
        name(std::shared_ptr<TmpVar> dest,                             \
             std::shared_ptr<Value> lhs,                               \
             std::shared_ptr<Value> rhs)                               \
            : InstrWithResult{dest}, lhs_{lhs}, rhs_{rhs} {            \
            assert(dest->type() == lhs->type());                       \
            assert(dest->type() == rhs->type());                       \
        }                                                              \
                                                                       \
    public:                                                            \
        static std::shared_ptr<name> get(Context& ctx,                 \
                                         std::shared_ptr<Value> lhs,   \
                                         std::shared_ptr<Value> rhs) { \
            return std::shared_ptr<name>{new name{                     \
                TmpVar::get(ctx, ctx.get##ty##Ty()),                   \
                lhs,                                                   \
                rhs,                                                   \
            }};                                                        \
        }                                                              \
        std::shared_ptr<Value> lhs() const {                           \
            return lhs_;                                               \
        }                                                              \
        std::shared_ptr<Value> rhs() const {                           \
            return rhs_;                                               \
        }                                                              \
    };

MAKE_BINARY_INSTR(AddInstr, Int);
MAKE_BINARY_INSTR(SubInstr, Int);
MAKE_BINARY_INSTR(MulInstr, Int);
MAKE_BINARY_INSTR(AndInstr, Bool);
MAKE_BINARY_INSTR(OrInstr, Bool);

#undef MAKE_BINARY_INSTR

class CmpInstr final : public InstrWithResult {
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

    CmpInstr(std::shared_ptr<TmpVar> dest,
             Cond cond,
             std::shared_ptr<Value> lhs,
             std::shared_ptr<Value> rhs);

public:
    static std::shared_ptr<CmpInstr> get(Context& ctx,
                                         Cond cond,
                                         std::shared_ptr<Value> lhs,
                                         std::shared_ptr<Value> rhs);

    Cond cond() const;
    std::shared_ptr<Value> lhs() const;
    std::shared_ptr<Value> rhs() const;
};

class GetFieldPtr final : public InstrWithResult {
    std::shared_ptr<Value> target_;
    unsigned fieldIdx_;

    GetFieldPtr(std::shared_ptr<TmpVar> dest,
                std::shared_ptr<Value> target,
                unsigned fieldIdx);

public:
    static std::shared_ptr<GetFieldPtr> get(Context& ctx,
                                            std::shared_ptr<Value> target,
                                            unsigned fieldIdx);
};

class GotoInstr final : public Instr {
    BasicBlock* dest_;

    GotoInstr(VoidType* ty, BasicBlock* dest);

public:
    static std::shared_ptr<GotoInstr> get(Context& ctx, BasicBlock* dest);

    bool isTerminator() const override;
};

class BrInstr final : public Instr {
    std::shared_ptr<Value> cond_;
    BasicBlock* onTrue_;
    BasicBlock* onFalse_;

    BrInstr(VoidType* ty,
            std::shared_ptr<Value> cond,
            BasicBlock* onTrue,
            BasicBlock* onFalse);

public:
    static std::shared_ptr<BrInstr> get(Context& ctx,
                                        std::shared_ptr<Value> cond,
                                        BasicBlock* onTrue,
                                        BasicBlock* onFalse);

    bool isTerminator() const override;
};

class RetInstr final : public Instr {
    std::optional<std::shared_ptr<Value>> value_;

    RetInstr(VoidType* ty, std::optional<std::shared_ptr<Value>> value);

public:
    static std::shared_ptr<RetInstr> get(
        Context& ctx,
        std::optional<std::shared_ptr<Value>> value = std::nullopt);

    std::optional<std::shared_ptr<Value>> value() const;

    bool isTerminator() const override;
};
}  // namespace ir