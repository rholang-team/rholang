#pragma once

#include <memory>
#include <string>

namespace lir {
struct Value {
    virtual ~Value() = default;
};

struct Address {
    virtual ~Address() = default;
};

struct Register : public Value, public Address {};

struct Instr {
    virtual ~Instr() = default;
};

class Immediate final : public Value {
    int value_;

public:
    Immediate(int value) : value_{value} {}

    int value() const {
        return value_;
    }
};

class AddressExpression final : public Value, public Address {
    int displacement_;

public:
    std::shared_ptr<Register> base;

    AddressExpression(std::shared_ptr<Register> base, int displacement)
        : displacement_{displacement}, base{base} {}

    int displacement() const {
        return displacement_;
    }
};

class StackSlot final : public Value, public Address {
    size_t slot_;

public:
    StackSlot(size_t slot) : slot_{slot} {}

    size_t slot() const {
        return slot_;
    }
};

class Global final : public Value, public Address {
    std::string name_;

public:
    template <typename T>
    Global(T&& name) : name_{std::forward<T>(name)} {}

    std::string_view name() const {
        return name_;
    }
};
}  // namespace lir
