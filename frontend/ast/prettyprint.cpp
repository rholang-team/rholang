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
    depth += 1;
    pad();
    os << "VarDecl " << decl.name.value << " " << *decl.type.value << '\n';
    visit(decl.value.get());
    depth -= 1;
}

void PrettyPrinter::visit(FunctionDecl& decl) {
    depth += 1;
    pad();
    os << "FunctionDecl " << decl.name.value << " (";
    bool first = true;
    for (const auto& [n, t] :
         std::ranges::zip_view{decl.paramNames, decl.paramTypes}) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << n << ": " << *t.value;
    }
    os << ") -> " << *decl.rettype.value << '\n';
    visit(decl.body);
    depth -= 1;
}

void PrettyPrinter::visit(StructDecl& decl) {
    depth += 1;
    pad();

    os << "StructDecl " << decl.name.value << '\n';

    depth += 1;
    for (const auto& [name, type] : decl.fields) {
        pad();
        os << name.value << ' ' << *type.value << '\n';
    }
    depth -= 1;

    if (!decl.fields.empty() && !decl.methods.empty()) {
        os << '\n';
    }

    for (auto& [_, decl] : decl.methods) {
        visit(decl);
    }

    depth -= 1;
}

void PrettyPrinter::visit(CompoundStmt& stmt) {
    pad();
    os << "CompoundStmt\n";
    for (auto& s : stmt.stmts) {
        visit(s.get());
    }
}

void PrettyPrinter::visit(CondStmt& stmt) {
    pad();
    os << "CondStmt\n";
    visit(stmt.cond.get());

    pad();
    os << "Then\n";
    visit(stmt.onTrue);

    if (stmt.onFalse.has_value()) {
        pad();
        os << "Else\n";
        visit(stmt.onFalse->get());
    }
}

void PrettyPrinter::visit(WhileStmt& stmt) {
    pad();
    os << "WhileStmt\n";
    visit(stmt.cond.get());

    visit(stmt.body);
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

void PrettyPrinter::visit(AssignmentStmt& stmt) {
    pad();
    os << "AssignmentStmt\n";
    visit(stmt.lhs.get());
    visit(stmt.rhs.get());
}

void PrettyPrinter::visit(UnaryExpr& expr) {
    pad();
    os << "UnaryExpr ";
    switch (expr.op.value) {
        case UnaryExpr::Op::Minus:
            os << '-';
            break;
        case UnaryExpr::Op::Not:
            os << '!';
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
        case BinaryExpr::Op::Eq:
            os << "==";
            break;
        case BinaryExpr::Op::Ne:
            os << "!=";
            break;
        case BinaryExpr::Op::Lt:
            os << "<";
            break;
        case BinaryExpr::Op::Gt:
            os << ">";
            break;
        case BinaryExpr::Op::Le:
            os << "<=";
            break;
        case BinaryExpr::Op::Ge:
            os << ">=";
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

    if (!expr.args.empty()) {
        pad();
        os << "Call arguments:\n";
        for (auto& a : expr.args) {
            visit(a.get());
        }
    }
}

void PrettyPrinter::visit(StructInitExpr& expr) {
    pad();
    os << "StructInitExpr ";
    showTyPtr(expr.type.get());
    os << '\n';
    for (const auto& [n, f] : expr.fields) {
        pad();
        os << n << '\n';
        visit(f.get());
    }
}
}  // namespace frontend::ast
