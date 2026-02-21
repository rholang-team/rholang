#pragma once

#include <memory>
#include <vector>

namespace ir {
struct Type {
    enum class Id {
        Bool,
        Int,
        Pointer,
        Function,
        Struct,
    };

    /// Funny thing inspired by LLVM.
    /// This may contain function parameter types, struct field types or even
    /// the type under the pointer.
    std::vector<std::shared_ptr<Type>> containedTypes;

    static const std::shared_ptr<Type> boolType;
    static const std::shared_ptr<Type> intType;

    static std::shared_ptr<Type> makePointer(std::shared_ptr<Type> underlying);

    static std::shared_ptr<Type> makeFunction(std::shared_ptr<Type> rettype, );
};
}  // namespace ir
