#include "compiler/lir/value.hpp"

namespace lir {
bool Immediate::operator==(const Value& that) const {
    auto p = dynamic_cast<const Immediate*>(&that);
    return p != nullptr && value_ == p->value_;
}

bool AddressExpression::operator==(const Value& that) const {
    auto p = dynamic_cast<const AddressExpression*>(&that);
    return p != nullptr && displacement_ == p->displacement_ &&
           *base == *p->base;
}

// bool StackSlot::operator==(const Value& that) const {
//     auto p = dynamic_cast<const StackSlot*>(&that);
//     return p != nullptr && slot_ == p->slot_;
// }

bool GlobalRef::operator==(const Value& that) const {
    auto p = dynamic_cast<const GlobalRef*>(&that);
    return p != nullptr && name_ == p->name_;
}
}  // namespace lir
