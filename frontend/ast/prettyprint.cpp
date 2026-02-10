#include "frontend/ast/prettyprint.hpp"

#include <ostream>
#include <ranges>

#include "frontend/ast/decl.hpp"
#include "frontend/ast/declstmt.hpp"
#include "frontend/ast/expr.hpp"
#include "frontend/ast/stmt.hpp"

namespace {
constexpr const char* PADDING = "  ";
}  // namespace

namespace frontend::ast {
void PrettyPrinter::showTyPtr(frontend::Type* ty) {
    if (ty == nullptr) {
        os << "?";
    } else {
        os << *ty;
    }
}

void PrettyPrinter::pad() {
    for (unsigned i = 1; i < depth; ++i) {
        os << PADDING;
    }
}

void PrettyPrinter::visit(Decl* decl) {
    depth += 1;
    DeclVisitor<void>::visit(decl);
    depth -= 1;
}

void PrettyPrinter::visit(Stmt* stmt) {
    depth += 1;
    StmtVisitor<void>::visit(stmt);
    depth -= 1;
}

void PrettyPrinter::visit(Expr* expr) {
    depth += 1;
    ExprVisitor<void>::visit(expr);
    depth -= 1;
}

void PrettyPrinter::visit(VarDecl& decl) {
    pad();
    os << "VarDecl " << decl.name.value << " " << *decl.type.value << '\n';
    if (decl.value.has_value()) {
        visit(decl.value->get());
    }
}

void PrettyPrinter::visit(FunctionDecl& decl) {
    pad();
    os << "FunctionDecl " << decl.name.value << " (";
    bool first = true;
    for (const auto& [n, t] : decl.params) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << n.value << ": " << *t.value;
    }
    os << ") -> " << *decl.rettype.value << '\n';
    visit(decl.body);
}

void PrettyPrinter::visit(CompoundStmt& stmt) {
    pad();
    os << "CompoundStmt\n";
    for (auto& s : stmt.stmts) {
        visit(s.get());
    }
}

void PrettyPrinter::visit(RetStmt& stmt) {
    pad();
    os << "RetStmt\n";
    if (stmt.value.has_value()) {
        visit(stmt.value->get());
    }
}

void PrettyPrinter::visit(DeclStmt& stmt) {
    pad();
    os << "DeclStmt\n";
    visit(stmt.decl);
}

void PrettyPrinter::visit(ExprStmt& stmt) {
    pad();
    os << "ExprStmt\n";
    visit(stmt.expr.get());
}

void PrettyPrinter::visit(UnaryExpr& expr) {
    pad();
    os << "UnaryExpr ";
    switch (expr.op.value) {
        case UnaryExpr::Op::Minus:
            os << '-';
            break;
    }
    os << ' ';
    showTyPtr(expr.type.get());
    os << '\n';
    visit(expr.value.get());
}

void PrettyPrinter::visit(BinaryExpr& expr) {
    pad();
    os << "BinaryExpr ";
    switch (expr.op.value) {
        case BinaryExpr::Op::Assign:
            os << '=';
            break;
        case BinaryExpr::Op::Eq:
            os << "==";
            break;
        case BinaryExpr::Op::Plus:
            os << '+';
            break;
        case BinaryExpr::Op::Minus:
            os << '-';
            break;
        case BinaryExpr::Op::Mul:
            os << '*';
            break;
    }
    os << ' ';
    showTyPtr(expr.type.get());
    os << '\n';
    visit(expr.lhs.get());
    visit(expr.rhs.get());
}

void PrettyPrinter::visit(NumLitExpr& expr) {
    pad();
    os << "NumLitExpr " << expr.value.value << ' ';
    showTyPtr(expr.type.get());
    os << '\n';
}

void PrettyPrinter::visit(VarRefExpr& expr) {
    pad();
    os << "VarRefExpr " << expr.name.value << ' ';
    showTyPtr(expr.type.get());
    os << '\n';
}

void PrettyPrinter::visit(MemberRefExpr& expr) {
    pad();
    os << "MemberRefExpr " << expr.member.value << ' ';
    showTyPtr(expr.type.get());
    os << '\n';
    visit(expr.target.get());
}

void PrettyPrinter::visit(CallExpr& expr) {
    pad();
    os << "CallExpr ";
    showTyPtr(expr.type.get());
    os << '\n';
    visit(expr.callee.get());
    pad();
    os << "Call arguments:\n";
    for (auto& a : expr.args) {
        visit(a.get());
    }
}
}  // namespace frontend::ast
