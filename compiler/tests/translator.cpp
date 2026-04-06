#include <gtest/gtest.h>

#include <print>

#include "compiler/frontend/ast2ir.hpp"
#include "compiler/frontend/lex/lexer.hpp"
#include "compiler/frontend/parse/parser.hpp"
#include "compiler/frontend/sema.hpp"
#include "compiler/ir/builder.hpp"
#include "compiler/ir/module.hpp"
#include "compiler/ir/prettyprint.hpp"

namespace {
ir::Module translate(ir::Context& ctx, std::string_view input) {
    frontend::lex::Lexer lexer{std::move(input)};
    frontend::lex::Lexemes lexemes{lexer.lex()};
    frontend::parse::Parser parser{std::move(lexemes)};

    frontend::ast::File file;
    try {
        file = parser.parse();
    } catch (const frontend::Error& e) {
        std::print(stderr, "syntax error: {}", e.pretty());
        throw e;
    }

    frontend::TranslationUnit tu;
    try {
        tu = frontend::runSema(std::move(file));
    } catch (const frontend::Error& e) {
        std::print(stderr, "error: {}", e.pretty());
        throw e;
    }

    return frontend::ast2ir::translate(ctx, tu);
}

bool compareModules(ir::Module& expectedModule, ir::Module& actualModule) {
    if (expectedModule == actualModule) {
        return true;
    }

    std::println(stderr, "Module equality check failed.\nExpected:");
    {
        ir::PrettyPrinter pp{std::cerr};
        pp.visit(expectedModule);
    }
    std::println(stderr, "\nActual:");
    {
        ir::PrettyPrinter pp{std::cerr};
        pp.visit(actualModule);
    }
    return false;
}

ir::StructType* makeStructTy(ir::Builder& builder,
                             std::initializer_list<ir::Type*> fields) {
    std::vector<ir::Type*> fieldsVec{fields};
    return builder.structTy(fieldsVec);
}
}  // namespace

TEST(Translator, NestedOperators) {
    std::string input = R"(
            fun foo(a int, b int) int {
                return a + b - 4 * a / (b - 42) * 37 * (2 - a);
            }
        )";

    ir::Context ctx;
    ir::Module expectedModule;

    {
        ir::Builder builder(ctx);

        std::vector<ir::Type*> params{builder.intTy(), builder.intTy()};

        builder.startFunction("foo",
                              builder.functionTy(builder.intTy(), params));

        builder.startBb();

        auto x0 = builder.addToCurBb(
            builder.addInstr(builder.fnArgRef(0), builder.fnArgRef(1)));

        auto x1 = builder.addToCurBb(
            builder.mulInstr(builder.intImm(4), builder.fnArgRef(0)));

        auto x2 = builder.addToCurBb(
            builder.subInstr(builder.fnArgRef(1), builder.intImm(42)));

        auto x3 = builder.addToCurBb(builder.divInstr(x1, x2));

        auto x4 = builder.addToCurBb(builder.mulInstr(x3, builder.intImm(37)));

        auto x5 = builder.addToCurBb(
            builder.subInstr(builder.intImm(2), builder.fnArgRef(0)));

        auto x6 = builder.addToCurBb(builder.mulInstr(x4, x5));

        auto x7 = builder.addToCurBb(builder.subInstr(x0, x6));

        builder.addToCurBb(builder.retInstr(x7));

        builder.finishBb();
        builder.finishFunction();

        expectedModule = builder.build();
    }

    ir::Module actualModule;
    ASSERT_NO_THROW(actualModule = translate(ctx, input));

    ASSERT_TRUE(compareModules(expectedModule, actualModule));
}

TEST(Translator, DeepAssignment) {
    std::string input = R"(
struct Foo {
    var x int;
    var bar Bar;
}

struct Bar {
    var baz Baz;
    var b bool;
}

struct Baz {
    var foo Foo;
    var qux Qux;
}

struct Qux {
    var c int;
    var plok Plok;
}

struct Plok {
    var b bool;
    var v int;
}

fun foo(arg Foo) void {
    arg.bar.baz.qux.plok.v = 42;
}
)";

    ir::Context ctx;
    ir::Module expectedModule;

    {
        ir::Builder builder(ctx);

        ir::StructType* fooTy =
            makeStructTy(builder, {builder.intTy(), builder.pointerTy()});
        ir::StructType* barTy =
            makeStructTy(builder, {builder.pointerTy(), builder.boolTy()});
        ir::StructType* bazTy =
            makeStructTy(builder, {builder.pointerTy(), builder.pointerTy()});
        ir::StructType* quxTy =
            makeStructTy(builder, {builder.intTy(), builder.pointerTy()});
        ir::StructType* plokTy =
            makeStructTy(builder, {builder.boolTy(), builder.intTy()});

        std::vector<ir::Type*> params{builder.pointerTy()};
        builder.startFunction("foo",
                              builder.functionTy(builder.voidTy(), params));

        builder.startBb();

        auto x0 = builder.addToCurBb(
            builder.getFieldPtrInstr(fooTy, builder.fnArgRef(0), 1));

        auto x1 =
            builder.addToCurBb(builder.loadInstr(builder.pointerTy(), x0));

        auto x2 = builder.addToCurBb(builder.getFieldPtrInstr(barTy, x1, 0));

        auto x3 =
            builder.addToCurBb(builder.loadInstr(builder.pointerTy(), x2));

        auto x4 = builder.addToCurBb(builder.getFieldPtrInstr(bazTy, x3, 1));

        auto x5 =
            builder.addToCurBb(builder.loadInstr(builder.pointerTy(), x4));

        auto x6 = builder.addToCurBb(builder.getFieldPtrInstr(quxTy, x5, 1));

        auto x7 =
            builder.addToCurBb(builder.loadInstr(builder.pointerTy(), x6));

        auto x8 = builder.addToCurBb(builder.getFieldPtrInstr(plokTy, x7, 1));

        builder.addToCurBb(
            builder.storeInstr(builder.intTy(), x8, builder.intImm(42)));

        builder.addToCurBb(builder.retInstr());
        builder.finishBb();
        builder.finishFunction();

        expectedModule = builder.build();
    }

    ir::Module actualModule;
    ASSERT_NO_THROW(actualModule = translate(ctx, input));

    ASSERT_TRUE(compareModules(expectedModule, actualModule));
}
