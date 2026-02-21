#pragma once

#include <cstddef>
#include <type_traits>
#include <unordered_set>

#include "compiler/ir/type.hpp"
namespace ir {
namespace internal {
struct PointerTypeHash {
    using is_transparent = std::true_type;

    size_t operator()(const PointerType*) const;
    size_t operator()(const PointerType&) const;
};

struct PointerTypeEq {
    using is_transparent = std::true_type;

    size_t operator()(const PointerType&, const PointerType&) const;
    size_t operator()(const PointerType*, const PointerType&) const;
    size_t operator()(const PointerType&, const PointerType*) const;
    size_t operator()(const PointerType*, const PointerType*) const;
};

struct StructTypeHash {
    using is_transparent = std::true_type;

    size_t operator()(const StructType*) const;
    size_t operator()(const StructType&) const;
};

struct StructTypeEq {
    using is_transparent = std::true_type;

    size_t operator()(const StructType&, const StructType&) const;
    size_t operator()(const StructType*, const StructType&) const;
    size_t operator()(const StructType&, const StructType*) const;
    size_t operator()(const StructType*, const StructType*) const;
};

struct FunctionTypeHash {
    using is_transparent = std::true_type;

    size_t operator()(const FunctionType*) const;
    size_t operator()(const FunctionType&) const;
};

struct FunctionTypeEq {
    using is_transparent = std::true_type;

    size_t operator()(const FunctionType&, const FunctionType&) const;
    size_t operator()(const FunctionType*, const FunctionType&) const;
    size_t operator()(const FunctionType&, const FunctionType*) const;
    size_t operator()(const FunctionType*, const FunctionType*) const;
};
}  // namespace internal

using PointerTypeSet = std::unordered_set<PointerType*,
                                          internal::PointerTypeHash,
                                          internal::PointerTypeEq>;

using StructTypeSet = std::unordered_set<StructType*,
                                         internal::StructTypeHash,
                                         internal::StructTypeEq>;

using FunctionTypeSet = std::unordered_set<FunctionType*,
                                           internal::FunctionTypeHash,
                                           internal::FunctionTypeEq>;
}  // namespace ir
