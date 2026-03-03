#include "compiler/frontend/ast/expr.hpp"

#include <algorithm>

namespace frontend::ast {
lex::Span UnaryExpr::span() const {
    auto b = op.span.begin;
    auto e = value->span().end;
    return lex::Span{b, e};
}

lex::Span BinaryExpr::span() const {
    auto b = lhs->span().begin;
    auto e = rhs->span().end;
    return lex::Span{b, e};
}

lex::Span NumLitExpr::span() const {
    return value.span;
}

lex::Span BoolLitExpr::span() const {
    return value.span;
}

lex::Span VarRefExpr::span() const {
    return name.span;
}

lex::Span MemberRefExpr::span() const {
    auto b = target->span().begin;
    auto e = member.span.end;
    return lex::Span{b, e};
}

lex::Span CallExpr::span() const {
    auto calleeSpan = callee->span();
    size_t e;
    if (args.empty()) {
        e = calleeSpan.end;
    } else {
        e = args.back()->span().end;
    }

    return lex::Span{calleeSpan.begin, e};
}

lex::Span StructInitExpr::span() const {
    size_t end = 0;

    for (const auto& [_, field] : fields) {
        end = std::max(end, field->span().end);
    }

    return lex::Span{tySpan.begin, end};
}
}  // namespace frontend::ast
