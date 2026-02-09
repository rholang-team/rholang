#pragma once

#include <memory>

#include "frontend/ast/stmt.hpp"
#include "frontend/lex/span.hpp"
#include "frontend/type.hpp"

namespace frontend::ast {
struct Expr : public Stmt {
    std::shared_ptr<Type> type;

    Expr() : type{nullptr} {}
    explicit Expr(std::shared_ptr<Type> type) : type{type} {}

    virtual lex::Span span() const = 0;
};

struct UnaryExpr final : public Expr {
    enum class Op {
        Minus,
    };

    lex::WithSpan<Op> op;
    std::unique_ptr<Expr> value;

    UnaryExpr(lex::WithSpan<Op> op, std::unique_ptr<Expr> value)
        : op{op}, value{std::move(value)} {}

    lex::Span span() const override;

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct BinaryExpr final : public Expr {
    enum class Op {
        Assign,
        Eq,
        Plus,
        Minus,
        Mul,
    };

    lex::WithSpan<Op> op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    BinaryExpr(lex::WithSpan<Op> op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {}

    lex::Span span() const override;

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct NumLitExpr final : public Expr {
    lex::WithSpan<size_t> value;

    explicit NumLitExpr(lex::WithSpan<size_t> value) : Expr{PrimitiveType::intType}, value{value} {}

    lex::Span span() const override;

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct VarRefExpr final : public Expr {
    lex::WithSpan<std::string> name;

    explicit VarRefExpr(lex::WithSpan<std::string> name) : name{std::move(name)} {}

    lex::Span span() const override;

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct MemberRefExpr final : public Expr {
    std::unique_ptr<Expr> target;
    lex::WithSpan<std::string> member;

    MemberRefExpr(std::unique_ptr<Expr> target, lex::WithSpan<std::string> member)
        : target{std::move(target)}, member{std::move(member)} {}

    lex::Span span() const override;

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct CallExpr final : public Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args)
        : callee{std::move(callee)}, args{std::move(args)} {}

    lex::Span span() const override;

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};
}  // namespace frontend::ast
