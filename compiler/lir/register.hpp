#pragma once

#include <array>
#include <cstdint>
#include <utility>

#include "compiler/lir/value.hpp"

namespace lir {
enum class WordType : uint8_t {
    Byte,
    Word,
    Dword,
    Qword,
};

std::string_view wordTypeToString(WordType w);
size_t wordTypeToSize(WordType w);

struct StackPointer final : public Register {
    bool operator==(const Value& that) const override;

    static std::shared_ptr<StackPointer> create() {
        return std::make_shared<StackPointer>();
    }
};

class VirtualRegister final : public Register {
public:
    using Id = size_t;

    Id id() const {
        return id_;
    }

    bool operator==(const Value& that) const override;

    static std::shared_ptr<VirtualRegister> create(VirtualRegister::Id id) {
        return std::shared_ptr<VirtualRegister>{new VirtualRegister(id)};
    }

private:
    Id id_;

    VirtualRegister(Id id) : id_{id} {}
};

class VirtualRegisterFactory {
    VirtualRegister::Id counter_ = 0;

    VirtualRegister::Id nextId() {
        return counter_++;
    }

public:
    VirtualRegisterFactory() = default;

    VirtualRegister::Id created() const {
        return counter_;
    }

    void reset() {
        counter_ = 0;
    }

    std::shared_ptr<VirtualRegister> nextShared() {
        return VirtualRegister::create(nextId());
    }
};

class PhysicalRegister final : public Register {
public:
    enum class Name : uint8_t {
        Rax,
        // Rbx,
        Rcx,
        Rdx,
        Rsi,
        Rdi,
        R8,
        R9,
        R10,
        R11,
        // R12,
        // R13,
        // R14,
        // R15,
    };

private:
    Name name_;

    PhysicalRegister(Name name) : name_{name} {}

public:
    static constexpr std::array<Name, 9> names{
        Name::Rax,
        // Name::Rbx,
        Name::Rcx,
        Name::Rdx,
        Name::Rsi,
        Name::Rdi,
        Name::R8,
        Name::R9,
        Name::R10,
        Name::R11,
        // Name::R12,
        // Name::R13,
        // Name::R14,
        // Name::R15,
    };

    static std::shared_ptr<PhysicalRegister> create(Name name) {
        return std::shared_ptr<PhysicalRegister>{new PhysicalRegister(name)};
    }

    Name name() const {
        return name_;
    }

    static std::string_view nameToString(Name name, WordType word);

    std::string_view toString(WordType word) const {
        return nameToString(name_, word);
    }

    bool operator==(const Value& that) const override;
};
}  // namespace lir

template <>
struct std::hash<lir::PhysicalRegister> {
    size_t operator()(const lir::PhysicalRegister& r) const {
        return std::to_underlying(r.name());
    }
};

template <>
struct std::hash<lir::VirtualRegister> {
    size_t operator()(const lir::VirtualRegister& r) const {
        return r.id();
    }
};
