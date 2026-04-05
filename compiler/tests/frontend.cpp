#include <gtest/gtest.h>

#include <algorithm>
#include <print>
#include <ranges>

#include "compiler/frontend/lex/lexer.hpp"
#include "compiler/frontend/parse/parser.hpp"
#include "compiler/frontend/sema.hpp"

using namespace frontend;

namespace {
TranslationUnit runFrontend(std::string_view input) {
    lex::Lexer lexer{std::move(input)};
    lex::Lexemes lexemes{lexer.lex()};
    parse::Parser parser{std::move(lexemes)};

    ast::File file;
    try {
        file = parser.parse();
    } catch (const frontend::Error& e) {
        std::print(stderr, "syntax error: {}", e.pretty());
        throw e;
    }

    try {
        return frontend::runSema(std::move(file));
    } catch (const frontend::Error& e) {
        std::print(stderr, "error: {}", e.pretty());
        throw e;
    }
}

template <typename Derived, typename Base>
    requires std::derived_from<Derived, Base>
const Derived& cast(const Base& base) {
    return dynamic_cast<const Derived&>(base);
}

template <typename T>
struct Checker {
    virtual ~Checker() = default;

    virtual void run(const T& x) const = 0;
};

struct IntTypeChecker final : public Checker<Type> {
    void run(const Type& t) const override {
        EXPECT_TRUE(cast<PrimitiveType>(t).kind ==
                    PrimitiveType::Primitive::Int);
    }
};

struct VoidTypeChecker final : public Checker<Type> {
    void run(const Type& t) const override {
        EXPECT_TRUE(cast<PrimitiveType>(t).kind ==
                    PrimitiveType::Primitive::Void);
    }
};

struct BoolTypeChecker final : public Checker<Type> {
    void run(const Type& t) const override {
        EXPECT_TRUE(cast<PrimitiveType>(t).kind ==
                    PrimitiveType::Primitive::Bool);
    }
};

struct TypeRefChecker final : public Checker<Type> {
    const char* name;

    TypeRefChecker(const char* name) : name{name} {}

    void run(const Type& t) const override {
        EXPECT_TRUE(cast<TypeRef>(t).name == name);
    }
};

struct CompoundStmtChecker final : public Checker<ast::Stmt> {
    std::vector<Checker<ast::Stmt>*> checkers;

    CompoundStmtChecker(std::vector<Checker<ast::Stmt>*> checkers)
        : checkers{std::move(checkers)} {}

    ~CompoundStmtChecker() override {
        std::ranges::for_each(checkers, [](auto* p) { delete p; });
    }

    void run(const ast::Stmt& stmt) const override {
        const auto& cstmt = cast<ast::CompoundStmt>(stmt);

        EXPECT_EQ(cstmt.stmts.size(), checkers.size());

        for (const auto& [s, c] : std::views::zip(cstmt.stmts, checkers)) {
            c->run(*s);
        }
    }
};

struct VoidReturnStmtChecker final : public Checker<ast::Stmt> {
    void run(const ast::Stmt& stmt) const override {
        ASSERT_FALSE(cast<ast::RetStmt>(stmt).value.has_value());
    }
};

struct ReturnStmtChecker final : public Checker<ast::Stmt> {
    Checker<ast::Expr>* checker;

    ReturnStmtChecker(Checker<ast::Expr>* checker) : checker{checker} {}

    ~ReturnStmtChecker() override {
        delete checker;
    }

    void run(const ast::Stmt& stmt) const override {
        const auto& r = cast<ast::RetStmt>(stmt);

        ASSERT_TRUE(r.value.has_value());
        checker->run(**cast<ast::RetStmt>(stmt).value);
    }
};

struct AssignmentStmtChecker final : public Checker<ast::Stmt> {
    Checker<ast::Expr>* lhs;
    Checker<ast::Expr>* rhs;

    AssignmentStmtChecker(Checker<ast::Expr>* lhs, Checker<ast::Expr>* rhs)
        : lhs{lhs}, rhs{rhs} {}

    ~AssignmentStmtChecker() override {
        delete lhs;
        delete rhs;
    }

    void run(const ast::Stmt& stmt) const override {
        const auto& a = cast<ast::AssignmentStmt>(stmt);

        lhs->run(*a.lhs);
        rhs->run(*a.rhs);
    }
};

struct BinaryExprChecker final : public Checker<ast::Expr> {
    ast::BinaryExpr::Op op;
    Checker<Type>* ty;
    Checker<ast::Expr>* lhs;
    Checker<ast::Expr>* rhs;

    BinaryExprChecker(ast::BinaryExpr::Op op,
                      Checker<Type>* ty,
                      Checker<ast::Expr>* lhs,
                      Checker<ast::Expr>* rhs)
        : op{op}, ty{ty}, lhs{lhs}, rhs{rhs} {}

    ~BinaryExprChecker() {
        delete ty;
        delete lhs;
        delete rhs;
    }

    void run(const ast::Expr& expr) const override {
        ty->run(*expr.type);

        const auto& binexpr = cast<ast::BinaryExpr>(expr);

        EXPECT_EQ(binexpr.op.value, op);
        lhs->run(*binexpr.lhs);
        rhs->run(*binexpr.rhs);
    }
};

struct VarRefExprChecker final : public Checker<ast::Expr> {
    const char* name;
    Checker<Type>* ty;

    ~VarRefExprChecker() {
        delete ty;
    }

    VarRefExprChecker(const char* name, Checker<Type>* ty)
        : name{name}, ty{std::move(ty)} {}

    void run(const ast::Expr& expr) const override {
        ty->run(*expr.type);

        const auto& vexpr = cast<ast::VarRefExpr>(expr);

        EXPECT_EQ(vexpr.name.value, name);
        ty->run(*vexpr.type);
    }
};

struct MemberRefExprChecker final : public Checker<ast::Expr> {
    const char* member;
    Checker<Type>* ty;
    Checker<ast::Expr>* target;

    ~MemberRefExprChecker() {
        delete target;
        delete ty;
    }

    MemberRefExprChecker(const char* member,
                         Checker<Type>* ty,
                         Checker<ast::Expr>* target)
        : member{member}, ty{ty}, target{target} {}

    void run(const ast::Expr& expr) const override {
        ty->run(*expr.type);

        const auto& mrexpr = cast<ast::MemberRefExpr>(expr);

        EXPECT_EQ(mrexpr.member.value, member);
        target->run(*mrexpr.target);
    }
};

struct NumLitExprChecker final : public Checker<ast::Expr> {
    unsigned value;

    NumLitExprChecker(unsigned value) : value{value} {}

    void run(const ast::Expr& expr) const override {
        IntTypeChecker{}.run(*expr.type);

        const auto& nlexpr = cast<ast::NumLitExpr>(expr);

        EXPECT_EQ(nlexpr.value.value, value);
    }
};

void function(const ast::FunctionDecl& fn,
              std::string_view name,
              std::initializer_list<std::pair<std::string_view, Checker<Type>*>>
                  paramPreds,
              Checker<Type>* rettypePred,
              CompoundStmtChecker bodyPred) {
    EXPECT_EQ(fn.name.value, name);
    ASSERT_EQ(fn.paramNames.size(), fn.paramTypes.size());
    ASSERT_EQ(fn.paramNames.size(), paramPreds.size());

    for (const auto& triple :
         std::views::zip(std::views::zip(fn.paramNames, fn.paramTypes),
                         paramPreds)) {
        const auto& [param, pred] = triple;
        const auto& [n, t] = param;
        const auto& [en, tp] = pred;

        EXPECT_EQ(n, en);
        tp->run(*t.value);

        delete tp;
    }

    rettypePred->run(*fn.rettype.value);
    delete rettypePred;

    bodyPred.run(fn.body);
}
}  // namespace

TEST(Frontend, NestedOperators) {
    std::string input = R"(
            fun foo(a int, b int) int {
                return a + b - 4 * a / (b - 42) * 37 * (2 - a);
            }
        )";

    TranslationUnit tu;
    ASSERT_NO_THROW(tu = runFrontend(input));

    EXPECT_TRUE(tu.globals.empty());
    EXPECT_TRUE(tu.structs.empty());

    ASSERT_EQ(1, tu.functions.size());
    ASSERT_TRUE(tu.functions.contains("foo"));

    const ast::FunctionDecl& fn = *tu.functions.at("foo");

    function(
        fn,
        "foo",
        {{"a", new IntTypeChecker()}, {"b", new IntTypeChecker()}},
        new IntTypeChecker(),
        CompoundStmtChecker(std::vector<Checker<ast::Stmt>*>{
            new ReturnStmtChecker(new BinaryExprChecker(
                ast::BinaryExpr::Op::Minus,
                new IntTypeChecker(),
                new BinaryExprChecker(
                    ast::BinaryExpr::Op::Plus,
                    new IntTypeChecker(),
                    new VarRefExprChecker("a", new IntTypeChecker()),
                    new VarRefExprChecker("b", new IntTypeChecker())),
                new BinaryExprChecker(
                    ast::BinaryExpr::Op::Mul,
                    new IntTypeChecker(),
                    new BinaryExprChecker(
                        ast::BinaryExpr::Op::Mul,
                        new IntTypeChecker(),
                        new BinaryExprChecker(
                            ast::BinaryExpr::Op::Div,
                            new IntTypeChecker(),
                            new BinaryExprChecker(
                                ast::BinaryExpr::Op::Mul,
                                new IntTypeChecker(),
                                new NumLitExprChecker(4),
                                new VarRefExprChecker("a",
                                                      new IntTypeChecker())),
                            new BinaryExprChecker(
                                ast::BinaryExpr::Op::Minus,
                                new IntTypeChecker(),
                                new VarRefExprChecker("b",
                                                      new IntTypeChecker()),
                                new NumLitExprChecker(42))),
                        new NumLitExprChecker(37)),
                    new BinaryExprChecker(
                        ast::BinaryExpr::Op::Minus,
                        new IntTypeChecker(),
                        new NumLitExprChecker(2),
                        new VarRefExprChecker("a", new IntTypeChecker())))))}));
}

TEST(Frontend, DeepAssignment) {
    std::string input = R"(
struct Foo {
    var bar Bar;
}

struct Bar {
    var baz Baz;
}

struct Baz {
    var qux Qux;
}

struct Qux {
    var plok Plok;
}

struct Plok {
    var v int;
}

fun foo(arg Foo) void {
    arg.bar.baz.qux.plok.v = 42;
}
)";

    TranslationUnit tu;
    ASSERT_NO_THROW(tu = runFrontend(input));

    const ast::FunctionDecl& fn = *tu.functions.at("foo");

    function(fn,
             "foo",
             {{"arg", new TypeRefChecker("Foo")}},
             new VoidTypeChecker(),
             CompoundStmtChecker(
                 std::vector<Checker<ast::Stmt>*>{new AssignmentStmtChecker(
                     new MemberRefExprChecker(
                         "v",
                         new IntTypeChecker(),
                         new MemberRefExprChecker(
                             "plok",
                             new TypeRefChecker("Plok"),
                             new MemberRefExprChecker(
                                 "qux",
                                 new TypeRefChecker("Qux"),
                                 new MemberRefExprChecker(
                                     "baz",
                                     new TypeRefChecker("Baz"),
                                     new MemberRefExprChecker(
                                         "bar",
                                         new TypeRefChecker("Bar"),
                                         new VarRefExprChecker(
                                             "arg",
                                             new TypeRefChecker("Foo"))))))),
                     new NumLitExprChecker(42))}));
}
