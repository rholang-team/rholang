#include "frontend/type.hpp"

#include "utils/match.hpp"

namespace frontend {
const std::shared_ptr<Type> PrimitiveType::voidType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Void);

const std::shared_ptr<Type> PrimitiveType::boolType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Bool);

const std::shared_ptr<Type> PrimitiveType::intType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Int);

bool PrimitiveType::operator==(const PrimitiveType& that) const {
    return kind == that.kind;
}

bool TypeRef::operator==(const TypeRef& that) const {
    return name == that.name;
}

bool FunctionType::operator==(const FunctionType& that) const {
    return std::ranges::equal(
               params,
               that.params,
               [](const auto& x, const auto& y) { return *x == *y; }) &&
           *rettype == *that.rettype;
}

bool StructType::Field::operator==(const Field& that) const {
    return name == that.name && *type == *that.type;
}

bool StructType::operator==(const StructType& that) const {
    return name == that.name && std::ranges::equal(fields, that.fields);
}

bool Type::operator==(const Type& that) const {
    return utils::polymorphicEq<PrimitiveType>(this, &that) ||
           utils::polymorphicEq<FunctionType>(this, &that) ||
           utils::polymorphicEq<TypeRef>(this, &that) ||
           utils::polymorphicEq<StructType>(this, &that);
}
}  // namespace frontend

std::ostream& operator<<(std::ostream& os, const frontend::Type& ty) {
    std::format_to(std::ostreambuf_iterator{os}, "{}", ty);
    return os;
}

std::ostream& operator<<(std::ostream& os, const frontend::PrimitiveType& ty) {
    std::format_to(std::ostreambuf_iterator{os}, "{}", ty);
    return os;
}

std::ostream& operator<<(std::ostream& os, const frontend::TypeRef& ty) {
    std::format_to(std::ostreambuf_iterator{os}, "{}", ty);
    return os;
}

std::ostream& operator<<(std::ostream& os, const frontend::FunctionType& ty) {
    std::format_to(std::ostreambuf_iterator{os}, "{}", ty);
    return os;
}

std::ostream& operator<<(std::ostream& os, const frontend::StructType& ty) {
    std::format_to(std::ostreambuf_iterator{os}, "{}", ty);
    return os;
}
