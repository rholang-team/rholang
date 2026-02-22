#include "compiler/frontend/ast2ir.hpp"

#include "compiler/frontend/ast/visitor.hpp"
#include "compiler/ir/builder.hpp"

namespace frontend::ast2ir {
namespace {
class Translator : private ast::DeclVisitor,
                   ast::StmtVisitor<void>,
                   ast::ExprVisitor<void> {
    using DeclVisitor::visit;
    using StmtVisitor<void>::visit;
    using ExprVisitor<void>::visit;

    ir::Builder builder_;

public:
    Translator(ir::Context& ctx) : builder_{ctx} {}

    void visit(ast::FunctionDecl& decl) {}
    void visit(ast::VarDecl& decl) {}
    void visit(ast::StructDecl& decl) {}

    void visit(ast::CompoundStmt& stmt) {}
    void visit(ast::CondStmt& stmt) {}
    void visit(ast::WhileStmt& stmt) {}
    void visit(ast::DeclStmt& stmt) {}
    void visit(ast::RetStmt& stmt) {}
    void visit(ast::ExprStmt& stmt) {}
    void visit(ast::AssignmentStmt& stmt) {}

    void visit(ast::UnaryExpr& expr) {}
    void visit(ast::NumLitExpr& expr) {}
    void visit(ast::BinaryExpr& expr) {}
    void visit(ast::VarRefExpr& expr) {}
    void visit(ast::MemberRefExpr& expr) {}
    void visit(ast::CallExpr& expr) {}
    void visit(ast::StructInitExpr& expr) {}

    ir::Module run(TranslationUnit& tu) {
        // TODO: translate globals

        for (auto& [_, fn] : tu.functions) {
            visit(*fn);
        }

        return builder_.build();
    }
};
}  // namespace

ir::Module translate(ir::Context& ctx, TranslationUnit& tu) {
    return Translator{ctx}.run(tu);
}
}  // namespace frontend::ast2ir
