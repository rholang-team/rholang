#include <fstream>
#include <iostream>
#include <print>

#include "compiler/frontend/ast/prettyprint.hpp"
#include "compiler/frontend/ast2ir.hpp"
#include "compiler/frontend/lex/lexer.hpp"
#include "compiler/frontend/parse/parser.hpp"
#include "compiler/frontend/sema.hpp"
#include "compiler/ir/context.hpp"
#include "compiler/ir/prettyprint.hpp"

using namespace std::string_view_literals;

namespace {
void printTranslationUnit(frontend::TranslationUnit& tu) {
    for (auto& [name, s] : tu.structs) {
        std::println("{}\n", *s);
    }
    for (auto& [name, decl] : tu.globals) {
        frontend::ast::PrettyPrinter{std::cout}.visit(decl);
        std::cout << '\n';
    }
    for (auto& [name, decl] : tu.functions) {
        frontend::ast::PrettyPrinter{std::cout}.visit(*decl);
        std::cout << '\n';
    }
}
}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println(stderr, "too few arguments");
        return EXIT_FAILURE;
    }

    std::string input;
    {
        std::ifstream file{argv[1]};
        input = std::string{std::istreambuf_iterator{file}, {}};
    }

    bool dumpAst = false;
    bool dumpIr = false;
    for (int i = 2; i < argc; ++i) {
        if (argv[i] == "--dump-ast"sv) {
            dumpAst = true;
        } else if (argv[i] == "--dump-ir"sv) {
            dumpIr = true;
        } else {
            std::println(stderr, "unknown option {}", argv[i]);
        }
    }

    frontend::lex::Lexer lexer{std::move(input)};
    frontend::lex::Lexemes lexemes{lexer.lex()};
    frontend::parse::Parser parser{std::move(lexemes)};

    frontend::ast::File file;
    try {
        file = parser.parse();
    } catch (const frontend::Error& e) {
        std::print(stderr, "syntax error: {}", e.pretty());
    }

    frontend::TranslationUnit tu;
    try {
        tu = frontend::runSema(std::move(file));
    } catch (const frontend::Error& e) {
        std::print(stderr, "error: {}", e.pretty());
    }

    if (dumpAst)
        printTranslationUnit(tu);

    ir::Context ctx;
    ir::Module module = frontend::ast2ir::translate(ctx, tu);

    if (dumpIr) {
        ir::PrettyPrinter irPretty{std::cout};
        irPretty.visit(module);
    }
}
