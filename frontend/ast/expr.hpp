#pragma once

#include <memory>
#include <unordered_map>

#include "frontend/lex/span.hpp"
#include "frontend/type.hpp"

namespace frontend::ast {
struct Expr {
    std::shared_ptr<Type> type;

    Expr() : type{nullptr} {}
    virtual ~Expr() = default;
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
};

struct NumLitExpr final : public Expr {
    lex::WithSpan<size_t> value;

    explicit NumLitExpr(lex::WithSpan<size_t> value) : value{value} {}

    lex::Span span() const override;
};

struct VarRefExpr final : public Expr {
    lex::WithSpan<std::string> name;

    explicit VarRefExpr(lex::WithSpan<std::string> name) : name{std::move(name)} {}

    lex::Span span() const override;
};

struct MemberRefExpr final : public Expr {
    std::unique_ptr<Expr> target;
    lex::WithSpan<std::string> member;

    MemberRefExpr(std::unique_ptr<Expr> target, lex::WithSpan<std::string> member)
        : target{std::move(target)}, member{std::move(member)} {}

    lex::Span span() const override;
};

struct CallExpr final : public Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args)
        : callee{std::move(callee)}, args{std::move(args)} {}

    lex::Span span() const override;
};

struct StructInitExpr final : public Expr {
    lex::Span tySpan;
    std::unordered_map<std::string, std::unique_ptr<Expr>> fields;

    StructInitExpr(lex::WithSpan<std::shared_ptr<Type>> ty,
                   std::unordered_map<std::string, std::unique_ptr<Expr>> fields)
        : Expr{ty.value}, tySpan{ty.span}, fields{std::move(fields)} {}

    lex::Span span() const override;
};
}  // namespace frontend::ast
