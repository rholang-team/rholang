#include "frontend/type.hpp"

namespace frontend {
const std::shared_ptr<Type> PrimitiveType::voidType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Void);

const std::shared_ptr<Type> PrimitiveType::boolType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Bool);

const std::shared_ptr<Type> PrimitiveType::intType =
    std::make_shared<PrimitiveType>(PrimitiveType::Primitive::Int);
}  // namespace frontend
