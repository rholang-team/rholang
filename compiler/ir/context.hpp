#pragma once

#include <memory_resource>

#include "compiler/ir/type.hpp"
#include "compiler/ir/typesets.hpp"

namespace ir {
class Context {
    std::pmr::monotonic_buffer_resource memoryResource_;
    std::pmr::polymorphic_allocator<> allocator_;

    VoidType voidTy_;
    BoolType boolTy_;
    IntType intTy_;

    PointerTypeSet pointerTypes_;
    StructTypeSet structTypes_;
    FunctionTypeSet functionTypes_;

    size_t temporaryCounter_ = 0;

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

    size_t nextTmpIdx();

    VoidType* getVoidTy();
    BoolType* getBoolTy();
    IntType* getIntTy();

    PointerType* findPointerType(const PointerType& ty);
    StructType* findStructType(const StructType& ty);
    FunctionType* findFunctionType(const FunctionType& ty);

    void insertPointerType(PointerType* ptrTy);
    void insertStructType(StructType* structTy);
    void insertFunctionType(FunctionType* fnTy);
};
}  // namespace ir
