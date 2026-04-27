#pragma once

#include <memory>
#include <string>

namespace lir {
struct Assemblable {
    virtual ~Assemblable() = default;
    virtual std::string toAsm() const = 0;
};

struct Value {
    virtual ~Value() = default;
};

struct Address {
    virtual ~Address() = default;
};

struct AssemblableValue : public Value, public Assemblable {};

struct Register : public Value, public Address {};

struct Instr : public Assemblable {};

class Immediate final : public AssemblableValue {
    int value_;

public:
    Immediate(int value) : value_{value} {}

    int value() const {
        return value_;
    }

    std::string toAsm() const override {
        return std::to_string(value_);
    }
};

class AddressExpression final : public AssemblableValue, public Address {
    int displacement_;

public:
    std::shared_ptr<Register> base;

    AddressExpression(std::shared_ptr<Register> base, int displacement)
        : displacement_{displacement}, base{base} {}

    std::string toAsm() const override {
        throw "todo";
    }

    int displacement() const {
        return displacement_;
    }
};

class StackSlot final : public AssemblableValue, public Address {
    size_t slot_;

public:
    StackSlot(size_t slot) : slot_{slot} {}

    std::string toAsm() const override {
        throw "todo";
    }

    size_t slot() const {
        return slot_;
    }
};
}  // namespace lir
