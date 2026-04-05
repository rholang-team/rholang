#pragma once

#include <memory_resource>
#include <span>

#include "compiler/ir/type.hpp"
#include "compiler/ir/typesets.hpp"

namespace ir {
class StructType;
class FunctionType;

class Context {
    std::pmr::monotonic_buffer_resource memoryResource_;
    std::pmr::polymorphic_allocator<> allocator_;

    VoidType voidTy_;
    BoolType boolTy_;
    IntType intTy_;
    PointerType pointerTy_;

    StructTypeSet structTypes_;
    FunctionTypeSet functionTypes_;

public:
    Context() : memoryResource_{}, allocator_{&memoryResource_} {}

    Context(const Context&) = delete;
    Context(Context&&) = delete;

    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    template <typename T>
    T* allocate() {
        return allocator_.allocate_object<T>();
    }

    template <typename T>
    std::span<T> allocate(size_t n) {
        return std::span<T>{allocator_.allocate_object<T>(n), n};
    }

    VoidType* getVoidTy();
    BoolType* getBoolTy();
    IntType* getIntTy();
    PointerType* getPointerTy();

    StructType* findStructType(const StructType& ty);
    FunctionType* findFunctionType(const FunctionType& ty);

    void insertStructType(StructType* structTy);
    void insertFunctionType(FunctionType* fnTy);
};
}  // namespace ir
