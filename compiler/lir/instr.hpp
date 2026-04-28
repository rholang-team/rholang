#pragma once

#include <optional>

#include "compiler/lir/bb.hpp"
#include "compiler/lir/value.hpp"

namespace lir {
enum class WordType : uint8_t {
    Byte,
    Word,
    Dword,
    Qword,
};

std::string_view wordTypeToString(WordType w);

struct MovInstr final : public Instr {
    std::shared_ptr<Register> dest;
    std::shared_ptr<Register> src;

    MovInstr(std::shared_ptr<Register> dest, std::shared_ptr<Register> src)
        : dest{dest}, src{src} {}
};

class PushInstr final : public Instr {
    std::shared_ptr<Register> reg_;

public:
    std::shared_ptr<Register> reg() const {
        return reg_;
    }
};

class PopInstr final : public Instr {
    std::shared_ptr<Register> reg_;

public:
    std::shared_ptr<Register> reg() const {
        return reg_;
    }
};

struct LeaInstr final : public Instr {
    std::shared_ptr<Register> dest;
    std::shared_ptr<Address> addr;

    LeaInstr(std::shared_ptr<Register> dest, std::shared_ptr<Address> addr)
        : dest{dest}, addr{addr} {}
};

class LoadInstr final : public Instr {
    WordType itemSize_;

public:
    std::shared_ptr<Register> dest;
    std::shared_ptr<Address> src;

    LoadInstr(WordType itemSize,
              std::shared_ptr<Register> dest,
              std::shared_ptr<Address> src)
        : itemSize_{itemSize}, dest{dest}, src{src} {}

    WordType itemSize() const {
        return itemSize_;
    }
};

class LoadImmInstr final : public Instr {
private:
    int imm_;

public:
    std::shared_ptr<Register> dest;

    LoadImmInstr(std::shared_ptr<Register> dest, int imm)
        : imm_{imm}, dest{dest} {}

    int imm() const {
        return imm_;
    }
};

class StoreInstr final : public Instr {
    WordType itemSize_;

public:
    std::shared_ptr<Address> dest;
    std::shared_ptr<Register> src;

    StoreInstr(WordType itemSize,
               std::shared_ptr<Address> dest,
               std::shared_ptr<Register> src)
        : itemSize_{itemSize}, dest{dest}, src{src} {}

    WordType itemSize() const {
        return itemSize_;
    }
};

struct NotInstr final : public Instr {
    std::shared_ptr<Register> operand;

    NotInstr(std::shared_ptr<Register> operand) : operand{operand} {}
};

struct NegInstr final : public Instr {
    std::shared_ptr<Register> operand;

    NegInstr(std::shared_ptr<Register> operand) : operand{operand} {}
};

struct CmpInstr final : public Instr {
    enum class Cond {
        Eq,
        Ne,
        Lt,
        Le,
        Gt,
        Ge,
    };

    std::shared_ptr<Register> dest;
    Cond cond;
    std::shared_ptr<Register> lhs;
    std::shared_ptr<Register> rhs;

    CmpInstr(std::shared_ptr<Register> dest,
             Cond cond,
             std::shared_ptr<Register> lhs,
             std::shared_ptr<Register> rhs)
        : dest{dest}, cond{cond}, lhs{lhs}, rhs{rhs} {}
};

class JmpInstr final : public Instr {
    BasicBlock* dest_;

public:
    /// `bool` is for inverting the condition, meaning jmp will be on `!cond` if
    /// `invert` is set
    std::optional<std::pair<std::shared_ptr<Register>, bool>> cond;

    JmpInstr(BasicBlock* dest,
             std::optional<std::pair<std::shared_ptr<Register>, bool>> cond =
                 std::nullopt)
        : dest_{dest}, cond{cond} {}

    bool conditional() const {
        return cond.has_value();
    }

    bool invertedCond() const {
        return conditional() && cond->second;
    }

    BasicBlock* dest() const {
        return dest_;
    }
};

struct RetInstr final : public Instr {};

class CallInstr final : public Instr {
    std::string callee_;

public:
    std::optional<std::shared_ptr<Register>> dest;
    std::vector<std::shared_ptr<Register>> args;

    template <typename T>
    CallInstr(T&& callee, std::vector<std::shared_ptr<Register>> args)
        : callee_{std::forward<T>(callee)}, args{std::move(args)} {}

    template <typename T>
    CallInstr(T&& callee,
              std::optional<std::shared_ptr<Register>> dest,
              std::vector<std::shared_ptr<Register>> args)
        : callee_{std::forward<T>(callee)},
          dest{std::move(dest)},
          args{std::move(args)} {}

    std::string_view callee() const {
        return callee_;
    }
};

#define BINARY_INSTR(NAME)                      \
    struct NAME final : public Instr {          \
        std::shared_ptr<Register> dest;         \
        std::shared_ptr<Register> lhs;          \
        std::shared_ptr<Register> rhs;          \
                                                \
        NAME(std::shared_ptr<Register> dest,    \
             std::shared_ptr<Register> lhs,     \
             std::shared_ptr<Register> rhs)     \
            : dest{dest}, lhs{lhs}, rhs{rhs} {} \
    };

BINARY_INSTR(AddInstr);
BINARY_INSTR(SubInstr);
BINARY_INSTR(MulInstr);
BINARY_INSTR(DivInstr);

#undef BINARY_INSTR
}  // namespace lir