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

    ir::PrettyPrinter pp{std::cerr};
    std::println(stderr, "Module equality check failed.\nExpected:");
    pp.visit(expectedModule);
    std::println(stderr, "\nActual:");
    pp.visit(actualModule);
    return false;
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

        std::vector<ir::Type*> params{builder.getIntTy(), builder.getIntTy()};

        builder.startFunction(
            "foo",
            builder.getFunctionTy(builder.getIntTy(), params));

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
