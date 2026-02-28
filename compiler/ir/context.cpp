#include "compiler/ir/context.hpp"

namespace ir {
VoidType* Context::getVoidTy() {
    return &voidTy_;
}

BoolType* Context::getBoolTy() {
    return &boolTy_;
}

IntType* Context::getIntTy() {
    return &intTy_;
}

PointerType* Context::findPointerType(const PointerType& ty) {
    auto it = pointerTypes_.find(ty);
    return it == pointerTypes_.end() ? nullptr : *it;
}

StructType* Context::findStructType(const StructType& ty) {
    auto it = structTypes_.find(ty);
    return it == structTypes_.end() ? nullptr : *it;
}

FunctionType* Context::findFunctionType(const FunctionType& ty) {
    auto it = functionTypes_.find(ty);
    return it == functionTypes_.end() ? nullptr : *it;
}

void Context::insertPointerType(PointerType* ptrTy) {
    pointerTypes_.insert(ptrTy);
}

void Context::insertStructType(StructType* structTy) {
    structTypes_.insert(structTy);
}

void Context::insertFunctionType(FunctionType* fnTy) {
    functionTypes_.insert(fnTy);
}
}  // namespace ir
