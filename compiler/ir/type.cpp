#include "compiler/ir/type.hpp"

#include "compiler/ir/context.hpp"
#include "utils/match.hpp"

namespace ir {
VoidType* Type::getVoidTy(Context& c) {
    return c.getVoidTy();
}

BoolType* Type::getBoolTy(Context& c) {
    return c.getBoolTy();
}

IntType* Type::getIntTy(Context& c) {
    return c.getIntTy();
}

PointerType* Type::getPointerTy(Context& c) {
    return c.getPointerTy();
}

bool Type::isVoid() {
    return utils::isa<VoidType>(this);
}

StructType* StructType::get(Context& ctx, std::span<Type*> fields) {
    if (StructType* found = ctx.findStructType(StructType{fields})) {
        return found;
    }

    std::span<Type*> newFields = ctx.allocate<Type*>(fields.size());
    std::ranges::copy(fields, newFields.begin());

    StructType* res = ctx.allocate<StructType>();
    new (res) StructType{newFields};

    ctx.insertStructType(res);
    return res;
}

FunctionType* FunctionType::get(Context& ctx,
                                Type* rettype,
                                std::span<Type*> params) {
    if (FunctionType* found =
            ctx.findFunctionType(FunctionType{rettype, params})) {
        return found;
    }

    std::span<Type*> newParams = ctx.allocate<Type*>(params.size());
    std::ranges::copy(params, newParams.begin());

    FunctionType* res = ctx.allocate<FunctionType>();
    new (res) FunctionType{rettype, newParams};

    ctx.insertFunctionType(res);
    return res;
}
}  // namespace ir

#define IMPL_OSTREAM(T)                                        \
    std::ostream& operator<<(std::ostream& os, const T& x) {   \
        std::format_to(std::ostreambuf_iterator{os}, "{}", x); \
        return os;                                             \
    }

IMPL_OSTREAM(ir::Type)
IMPL_OSTREAM(ir::VoidType)
IMPL_OSTREAM(ir::IntType)
IMPL_OSTREAM(ir::BoolType)
IMPL_OSTREAM(ir::PointerType)
IMPL_OSTREAM(ir::StructType)
IMPL_OSTREAM(ir::FunctionType)
