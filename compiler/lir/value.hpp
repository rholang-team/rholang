#pragma once

#include <memory>
#include <string>

namespace lir {
struct Value {
    virtual ~Value() = default;

    virtual bool operator==(const Value& that) const = 0;
};

struct Address {
    virtual ~Address() = default;
};

struct Register : public Value, public Address {};

class Immediate final : public Value {
    int value_;

    Immediate(int value) : value_{value} {}

public:
    static std::shared_ptr<Immediate> create(int value) {
        return std::shared_ptr<Immediate>{new Immediate(value)};
    }

    int value() const {
        return value_;
    }

    bool operator==(const Value& that) const override;
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

    bool operator==(const Value& that) const override;
};

// class StackSlot final : public Value, public Address {
//     size_t slot_;

// public:
//     StackSlot(size_t slot) : slot_{slot} {}

//     size_t slot() const {
//         return slot_;
//     }

//     bool operator==(const Value& that) const override;
// };

class GlobalRef final : public Value, public Address {
    std::string name_;

public:
    template <typename T>
    GlobalRef(T&& name) : name_{std::forward<T>(name)} {}

    std::string_view name() const {
        return name_;
    }

    bool operator==(const Value& that) const override;
};
}  // namespace lir
