#include "frontend/pretty.hpp"

#include <ostream>
#include <ranges>
#include <sstream>

#include "frontend/ast/decl.hpp"
#include "frontend/ast/declstmt.hpp"
#include "frontend/ast/expr.hpp"
#include "frontend/ast/retstmt.hpp"
#include "frontend/ast/stmt.hpp"
#include "frontend/pretty.hpp"

namespace {
constexpr const char* PADDING = "  ";
}

namespace frontend::pretty {
void PrettyPrintable::pad(std::ostream& os, unsigned depth) const {
    for (unsigned i = 0; i < depth; ++i) {
        os << PADDING;
    }
}

std::string PrettyPrintable::pretty() const {
    std::stringstream ss{};
    pretty(ss);
    return ss.str();
}
}  // namespace frontend::pretty

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

namespace {
void showTyPtr(std::ostream& os, frontend::Type* ty) {
    if (ty == nullptr) {
        os << "?";
    } else {
        os << *ty;
    }
}
}  // namespace

namespace frontend::ast {
void VarDecl::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "VarDecl " << name.value << " " << *type.value << '\n';
    if (value.has_value()) {
        (*value)->pretty(os, depth + 1);
    }
}

void FunctionDecl::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "FunctionDecl " << name.value << " (";
    bool first = true;
    for (const auto& [n, t] : params) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << n.value << ": " << *t.value;
    }
    os << ") -> " << *rettype.value << '\n';
    body.pretty(os, depth + 1);
}

void CompoundStmt::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "CompoundStmt\n";
    for (const auto& s : stmts) {
        s->pretty(os, depth + 1);
    }
}

void RetStmt::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "RetStmt\n";
    if (value) {
        value->pretty(os, depth + 1);
    }
}

void DeclStmt::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "DeclStmt\n";
    decl.pretty(os, depth + 1);
}

void UnaryExpr::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "UnaryExpr ";
    switch (op.value) {
        case Op::Minus:
            os << '-';
            break;
    }
    os << ' ';
    showTyPtr(os, type.get());
    os << '\n';
    value->pretty(os, depth + 1);
}

void BinaryExpr::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "UnaryExpr ";
    switch (op.value) {
        case Op::Assign:
            os << '=';
            break;
        case Op::Eq:
            os << "==";
            break;
        case Op::Plus:
            os << '+';
            break;
        case Op::Minus:
            os << '-';
            break;
        case Op::Mul:
            os << '*';
            break;
    }
    os << ' ';
    showTyPtr(os, type.get());
    os << '\n';
    lhs->pretty(os, depth + 1);
    rhs->pretty(os, depth + 1);
}

void NumLitExpr::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "NumLitExpr " << value.value << ' ';
    showTyPtr(os, type.get());
    os << '\n';
}

void VarRefExpr::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "VarRefExpr " << name.value << ' ';
    showTyPtr(os, type.get());
    os << '\n';
}

void MemberRefExpr::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "MemberRefExpr " << member.value << ' ';
    showTyPtr(os, type.get());
    os << '\n';
    target->pretty(os, depth + 1);
}

void CallExpr::pretty(std::ostream& os, unsigned depth) const {
    pad(os, depth);
    os << "CallExpr ";
    showTyPtr(os, type.get());
    os << '\n';
    callee->pretty(os, depth + 1);
    pad(os, depth);
    os << "Call arguments:\n";
    for (const auto& a : args) {
        a->pretty(os, depth + 1);
    }
}
}  // namespace frontend::ast
