#pragma once

#include <cassert>
#include <memory>
#include <optional>

#include "compiler/ir/function.hpp"
#include "compiler/ir/value.hpp"

namespace ir {
class BasicBlock;

class Instr : public Value {
protected:
    explicit Instr(Type* type);

public:
    virtual bool isTerminator() const;
};

class AllocaInstr final : public Instr {
    explicit AllocaInstr(Type* ty);

public:
    static std::shared_ptr<AllocaInstr> create(Context& ctx, Type* itemType);

    Type* itemType() const;
};

class CallInstr final : public Instr {
    Function* callee_;
    std::vector<std::shared_ptr<Value>> args_;

    CallInstr(Function* callee, std::vector<std::shared_ptr<Value>> args);

public:
    static std::shared_ptr<CallInstr> create(
        Function* callee,
        std::vector<std::shared_ptr<Value>> args);

    Function* callee() const;
    std::vector<std::shared_ptr<Value>>& args();
    const std::vector<std::shared_ptr<Value>>& args() const;
};

class NotInstr final : public Instr {
    std::shared_ptr<Value> target_;

    NotInstr(std::shared_ptr<Value> target);

public:
    static std::shared_ptr<NotInstr> create(std::shared_ptr<Value> target);

    std::shared_ptr<Value> target() const;
};

class NegInstr final : public Instr {
    std::shared_ptr<Value> target_;

    NegInstr(std::shared_ptr<Value> target);

public:
    static std::shared_ptr<NegInstr> create(std::shared_ptr<Value> target);

    std::shared_ptr<Value> target() const;
};

class LoadInstr final : public Instr {
    std::shared_ptr<Value> src_;

    LoadInstr(Type* ty, std::shared_ptr<Value> src);

public:
    static std::shared_ptr<LoadInstr> create(std::shared_ptr<Value> src);

    std::shared_ptr<Value> src() const;
};

class StoreInstr final : public Instr {
    std::shared_ptr<Value> dest_;
    std::shared_ptr<Value> src_;

    StoreInstr(VoidType* ty,
               std::shared_ptr<Value> dest,
               std::shared_ptr<Value> src);

public:
    static std::shared_ptr<StoreInstr> create(Context& ctx,
                                              std::shared_ptr<Value> dest,
                                              std::shared_ptr<Value> src);

    std::shared_ptr<Value> dest() const;
    std::shared_ptr<Value> src() const;
};

#define MAKE_BINARY_INSTR(name, ty)                                       \
    class name final : public Instr {                                     \
        std::shared_ptr<Value> lhs_, rhs_;                                \
                                                                          \
        name(std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs)      \
            : Instr{lhs->type()}, lhs_{lhs}, rhs_{rhs} {                  \
            assert(lhs->type() == rhs->type());                           \
        }                                                                 \
                                                                          \
    public:                                                               \
        static std::shared_ptr<name> create(std::shared_ptr<Value> lhs,   \
                                            std::shared_ptr<Value> rhs) { \
            return std::shared_ptr<name>{new name{                        \
                lhs,                                                      \
                rhs,                                                      \
            }};                                                           \
        }                                                                 \
        std::shared_ptr<Value> lhs() const {                              \
            return lhs_;                                                  \
        }                                                                 \
        std::shared_ptr<Value> rhs() const {                              \
            return rhs_;                                                  \
        }                                                                 \
    };

MAKE_BINARY_INSTR(AddInstr, Int);
MAKE_BINARY_INSTR(SubInstr, Int);
MAKE_BINARY_INSTR(MulInstr, Int);

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

    CmpInstr(BoolType* ty,
             Cond cond,
             std::shared_ptr<Value> lhs,
             std::shared_ptr<Value> rhs);

public:
    static std::shared_ptr<CmpInstr> create(Context& ctx,
                                            Cond cond,
                                            std::shared_ptr<Value> lhs,
                                            std::shared_ptr<Value> rhs);

    Cond cond() const;
    std::shared_ptr<Value> lhs() const;
    std::shared_ptr<Value> rhs() const;
};

class GetFieldPtrInstr final : public Instr {
    std::shared_ptr<Value> target_;
    unsigned fieldIdx_;

    GetFieldPtrInstr(Type* ty,
                     std::shared_ptr<Value> target,
                     unsigned fieldIdx);

public:
    static std::shared_ptr<GetFieldPtrInstr> create(
        std::shared_ptr<Value> target,
        unsigned fieldIdx);

    std::shared_ptr<Value> target() const;
    unsigned fieldIdx() const;
};

class GotoInstr final : public Instr {
    BasicBlock* dest_;

    GotoInstr(VoidType* ty, BasicBlock* dest);

public:
    static std::shared_ptr<GotoInstr> create(Context& ctx, BasicBlock* dest);

    bool isTerminator() const override;

    BasicBlock* dest() const;
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
    static std::shared_ptr<BrInstr> create(Context& ctx,
                                           std::shared_ptr<Value> cond,
                                           BasicBlock* onTrue,
                                           BasicBlock* onFalse);

    bool isTerminator() const override;

    std::shared_ptr<Value> cond() const;
    BasicBlock* onTrue() const;
    BasicBlock* onFalse() const;
};

class RetInstr final : public Instr {
    std::optional<std::shared_ptr<Value>> value_;

    RetInstr(VoidType* ty, std::optional<std::shared_ptr<Value>> value);

public:
    static std::shared_ptr<RetInstr> create(
        Context& ctx,
        std::optional<std::shared_ptr<Value>> value = std::nullopt);

    std::optional<std::shared_ptr<Value>> value() const;

    bool isTerminator() const override;
};
}  // namespace ir