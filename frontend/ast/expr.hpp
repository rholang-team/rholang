#pragma once

#include <memory>

#include "frontend/ast/stmt.hpp"
#include "frontend/type.hpp"

namespace frontend::ast {
struct Expr : public Stmt {
    std::shared_ptr<Type> type;

    Expr() : type{nullptr} {}
    explicit Expr(std::shared_ptr<Type> type) : type{type} {}
};

struct UnaryExpr final : public Expr {
    enum class Op {
        Minus,
    };

    Op op;
    std::unique_ptr<Expr> value;

    UnaryExpr(Op op, std::unique_ptr<Expr> value) : op{op}, value{std::move(value)} {}

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

    Op op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    BinaryExpr(Op op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : op{op}, lhs{std::move(lhs)}, rhs{std::move(rhs)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct NumLitExpr final : public Expr {
    size_t value;

    explicit NumLitExpr(size_t value) : Expr{PrimitiveType::intType}, value{value} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct VarRefExpr final : public Expr {
    std::string name;

    template <std::constructible_from<std::string> S>
    explicit VarRefExpr(S&& name) : name{std::forward<S>(name)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct MemberRefExpr final : public Expr {
    std::unique_ptr<Expr> target;
    std::string member;

    template <std::constructible_from<std::string> S>
    MemberRefExpr(std::unique_ptr<Expr> target, S&& member)
        : target{std::move(target)}, member{std::forward<S>(member)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};

struct CallExpr final : public Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args)
        : callee{std::move(callee)}, args{std::move(args)} {}

    void pretty(std::ostream& os, unsigned depth = 0) const override;
};
}  // namespace frontend::ast
