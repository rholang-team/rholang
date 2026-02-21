#include "compiler/ir/typesets.hpp"

#include <algorithm>
#include <cstdint>

namespace {
template <typename T>
size_t ptrHash(const T* p) {
    return std::bit_cast<uintptr_t>(p) >> 3;
}
}  // namespace

namespace ir::internal {
size_t PointerTypeHash::operator()(const PointerType* ty) const {
    return operator()(*ty);
}
size_t PointerTypeHash::operator()(const PointerType& ty) const {
    return ptrHash(ty.underlying());
}
size_t PointerTypeEq::operator()(const PointerType& a,
                                 const PointerType& b) const {
    return a.underlying() == b.underlying();
}
size_t PointerTypeEq::operator()(const PointerType* a,
                                 const PointerType& b) const {
    return operator()(*a, b);
}
size_t PointerTypeEq::operator()(const PointerType& a,
                                 const PointerType* b) const {
    return operator()(a, *b);
}
size_t PointerTypeEq::operator()(const PointerType* a,
                                 const PointerType* b) const {
    return operator()(*a, *b);
}

size_t StructTypeHash::operator()(const StructType* ty) const {
    return operator()(*ty);
}
size_t StructTypeHash::operator()(const StructType& ty) const {
    size_t res = 1;
    for (const Type* field : ty.fields()) {
        res *= ptrHash(field);
    }

    return res;
}
size_t StructTypeEq::operator()(const StructType& a,
                                const StructType& b) const {
    return std::ranges::equal(a.fields(), b.fields());
}
size_t StructTypeEq::operator()(const StructType* a,
                                const StructType& b) const {
    return operator()(*a, b);
}
size_t StructTypeEq::operator()(const StructType& a,
                                const StructType* b) const {
    return operator()(a, *b);
}
size_t StructTypeEq::operator()(const StructType* a,
                                const StructType* b) const {
    return operator()(*a, *b);
}

size_t FunctionTypeHash::operator()(const FunctionType* ty) const {
    return operator()(*ty);
}
size_t FunctionTypeHash::operator()(const FunctionType& ty) const {
    size_t res = ptrHash(ty.rettype());
    for (const Type* param : ty.params()) {
        res *= ptrHash(param);
    }
    return res;
}
size_t FunctionTypeEq::operator()(const FunctionType& a,
                                  const FunctionType& b) const {
    return a.rettype() == b.rettype() &&
           std::ranges::equal(a.params(), b.params());
}
size_t FunctionTypeEq::operator()(const FunctionType* a,
                                  const FunctionType& b) const {
    return operator()(*a, b);
}
size_t FunctionTypeEq::operator()(const FunctionType& a,
                                  const FunctionType* b) const {
    return operator()(a, *b);
}
size_t FunctionTypeEq::operator()(const FunctionType* a,
                                  const FunctionType* b) const {
    return operator()(*a, *b);
}
}  // namespace ir::internal
