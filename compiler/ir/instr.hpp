#pragma once

#include <cassert>
#include <memory>
#include <optional>

#include "compiler/ir/function.hpp"
#include "compiler/ir/value.hpp"
#include "utils/match.hpp"

namespace ir {
class BasicBlock;

class Instr : public Value {
protected:
    explicit Instr(Type* type) : Value{type} {}

public:
    virtual bool isTerminator() const {
        return false;
    }
};

class AllocaInstr final : public Instr {
    Type* itemType_;

    AllocaInstr(PointerType* resType, Type* itemType)
        : Instr{resType}, itemType_{itemType} {}

public:
    static std::shared_ptr<AllocaInstr> create(Context& ctx, Type* itemType);

    Type* itemType() const {
        return itemType_;
    }

    virtual bool operator==(const Value& that) const override;
};

class NewInstr final : public Instr {
    Type* itemType_;

    NewInstr(PointerType* resType, Type* itemType)
        : Instr{resType}, itemType_{itemType} {}

public:
    static std::shared_ptr<NewInstr> create(Context& ctx, Type* itemType);

    Type* itemType() const {
        return itemType_;
    }

    virtual bool operator==(const Value& that) const override;
};

class CallInstr final : public Instr {
    const FunctionSignature* callee_;
    std::vector<std::shared_ptr<Value>> args_;

    CallInstr(const FunctionSignature* callee,
              std::vector<std::shared_ptr<Value>> args)
        : Instr{callee->type()->rettype()},
          callee_{callee},
          args_{std::move(args)} {}

public:
    static std::shared_ptr<CallInstr> create(
        const FunctionSignature* callee,
        std::vector<std::shared_ptr<Value>> args);

    const FunctionSignature* callee() const {
        return callee_;
    }
    std::vector<std::shared_ptr<Value>>& args() {
        return args_;
    }
    const std::vector<std::shared_ptr<Value>>& args() const {
        return args_;
    }

    virtual bool operator==(const Value& that) const override;
};

class NotInstr final : public Instr {
    std::shared_ptr<Value> target_;

    NotInstr(std::shared_ptr<Value> target)
        : Instr{target->type()}, target_{target} {
        assert(utils::isa<BoolType>(target->type()));
    }

public:
    static std::shared_ptr<NotInstr> create(std::shared_ptr<Value> target);

    std::shared_ptr<Value> target() const {
        return target_;
    }

    virtual bool operator==(const Value& that) const override;
};

class NegInstr final : public Instr {
    std::shared_ptr<Value> target_;

    NegInstr(std::shared_ptr<Value> target)
        : Instr{target->type()}, target_{target} {
        assert(utils::isa<IntType>(target->type()));
    }

public:
    static std::shared_ptr<NegInstr> create(std::shared_ptr<Value> target);

    std::shared_ptr<Value> target() const {
        return target_;
    }

    virtual bool operator==(const Value& that) const override;
};

class LoadInstr final : public Instr {
    std::shared_ptr<Value> src_;

    LoadInstr(Type* ty, std::shared_ptr<Value> src) : Instr{ty}, src_{src} {}

public:
    static std::shared_ptr<LoadInstr> create(Type* ty,
                                             std::shared_ptr<Value> src);

    std::shared_ptr<Value> src() const {
        return src_;
    }

    virtual bool operator==(const Value& that) const override;
};

class StoreInstr final : public Instr {
    Type* storedValueType_;
    std::shared_ptr<Value> dest_;
    std::shared_ptr<Value> src_;

    StoreInstr(VoidType* resType,
               Type* ty,
               std::shared_ptr<Value> dest,
               std::shared_ptr<Value> src)
        : Instr{resType}, storedValueType_{ty}, dest_{dest}, src_{src} {}

public:
    static std::shared_ptr<StoreInstr> create(Context& ctx,
                                              Type* ty,
                                              std::shared_ptr<Value> dest,
                                              std::shared_ptr<Value> src);

    std::shared_ptr<Value> dest() const {
        return dest_;
    }
    std::shared_ptr<Value> src() const {
        return src_;
    }
    Type* storedValueType() const {
        return storedValueType_;
    }

    virtual bool operator==(const Value& that) const override;
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
                                                                          \
        virtual bool operator==(const Value& that) const override;        \
    };

MAKE_BINARY_INSTR(AddInstr, Int);
MAKE_BINARY_INSTR(SubInstr, Int);
MAKE_BINARY_INSTR(MulInstr, Int);
MAKE_BINARY_INSTR(DivInstr, Int);

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
             std::shared_ptr<Value> rhs)
        : Instr{ty}, cond_{cond}, lhs_{lhs}, rhs_{rhs} {
        assert(lhs->type() == rhs->type());
    }

public:
    static std::shared_ptr<CmpInstr> create(Context& ctx,
                                            Cond cond,
                                            std::shared_ptr<Value> lhs,
                                            std::shared_ptr<Value> rhs);

    Cond cond() const {
        return cond_;
    }
    std::shared_ptr<Value> lhs() const {
        return lhs_;
    }
    std::shared_ptr<Value> rhs() const {
        return rhs_;
    }

    virtual bool operator==(const Value& that) const override;
};

class GetFieldPtrInstr final : public Instr {
    StructType* structType_;
    std::shared_ptr<Value> target_;
    unsigned fieldIdx_;

    GetFieldPtrInstr(PointerType* resType,
                     StructType* structType,
                     std::shared_ptr<Value> target,
                     unsigned fieldIdx)
        : Instr{resType},
          structType_{structType},
          target_{target},
          fieldIdx_{fieldIdx} {}

public:
    static std::shared_ptr<GetFieldPtrInstr> create(
        Context& ctx,
        StructType* structType,
        std::shared_ptr<Value> target,
        unsigned fieldIdx);

    std::shared_ptr<Value> target() const {
        return target_;
    }
    unsigned fieldIdx() const {
        return fieldIdx_;
    }
    Type* fieldType() const {
        return structType_->fields()[fieldIdx_];
    }
    StructType* structType() const {
        return structType_;
    }

    virtual bool operator==(const Value& that) const override;
};

class ControlFlowInstr : public Instr {
    bool containsBackedge_;

protected:
    ControlFlowInstr(Type* ty, bool containsBackedge)
        : Instr{ty}, containsBackedge_{containsBackedge} {}

public:
    bool containsBackedge() const {
        return containsBackedge_;
    }
};

class GotoInstr final : public ControlFlowInstr {
    BasicBlock* dest_;

    GotoInstr(VoidType* ty, BasicBlock* dest, bool backedge)
        : ControlFlowInstr{ty, backedge}, dest_{dest} {}

public:
    static std::shared_ptr<GotoInstr> create(Context& ctx,
                                             BasicBlock* dest,
                                             bool backedge = false);

    bool isTerminator() const override {
        return true;
    }
    BasicBlock* dest() const {
        return dest_;
    }

    virtual bool operator==(const Value& that) const override;
};

class BrInstr final : public ControlFlowInstr {
    std::shared_ptr<Value> cond_;
    BasicBlock* onTrue_;
    BasicBlock* onFalse_;

    BrInstr(VoidType* ty,
            std::shared_ptr<Value> cond,
            BasicBlock* onTrue,
            BasicBlock* onFalse,
            bool containsBackedge)
        : ControlFlowInstr{ty, containsBackedge},
          cond_{cond},
          onTrue_{onTrue},
          onFalse_{onFalse} {}

public:
    static std::shared_ptr<BrInstr> create(Context& ctx,
                                           std::shared_ptr<Value> cond,
                                           BasicBlock* onTrue,
                                           BasicBlock* onFalse,
                                           bool containsBackedge = false);

    bool isTerminator() const override {
        return true;
    }
    std::shared_ptr<Value> cond() const {
        return cond_;
    }
    BasicBlock* onTrue() const {
        return onTrue_;
    }
    BasicBlock* onFalse() const {
        return onFalse_;
    }

    virtual bool operator==(const Value& that) const override;
};

class RetInstr final : public Instr {
    std::optional<std::shared_ptr<Value>> value_;

    RetInstr(VoidType* ty, std::optional<std::shared_ptr<Value>> value)
        : Instr{ty}, value_{value} {}

public:
    static std::shared_ptr<RetInstr> create(
        Context& ctx,
        std::optional<std::shared_ptr<Value>> value = std::nullopt);

    std::optional<std::shared_ptr<Value>> value() const {
        return value_;
    }
    bool isTerminator() const override {
        return true;
    }

    virtual bool operator==(const Value& that) const override;
};
}  // namespace ir