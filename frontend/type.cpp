#include "frontend/type.hpp"

#include "utils/match.hpp"

namespace frontend {
const std::shared_ptr<Type> PrimitiveType::voidType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Void);

const std::shared_ptr<Type> PrimitiveType::boolType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Bool);

const std::shared_ptr<Type> PrimitiveType::intType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Int);

namespace {
template <typename T>
bool polymorphicEq(const Type* a, const Type* b) {
    return (utils::isa<T>(a) && utils::isa<T>(b)) &&
           (*dynamic_cast<const T*>(a) == *dynamic_cast<const T*>(b));
}
}  // namespace

bool Type::operator==(const Type& that) const {
    return polymorphicEq<PrimitiveType>(this, &that) || polymorphicEq<FunctionType>(this, &that) ||
           polymorphicEq<TypeRef>(this, &that);
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
