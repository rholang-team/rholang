#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>

#include "compiler/backend/lowerir.hpp"
#include "compiler/frontend/ast/prettyprint.hpp"
#include "compiler/frontend/ast2ir.hpp"
#include "compiler/frontend/lex/lexer.hpp"
#include "compiler/frontend/parse/parser.hpp"
#include "compiler/frontend/sema.hpp"
#include "compiler/ir/context.hpp"
#include "compiler/ir/prettyprint.hpp"
#include "compiler/lir/module.hpp"
#include "compiler/lir/prettyprint.hpp"

using namespace std::string_view_literals;

namespace {
void dumpTranslationUnit(std::ostream& os, frontend::TranslationUnit& tu) {
    for (auto& [name, s] : tu.structs) {
        os << *s << '\n';
    }
    for (auto& [name, decl] : tu.globals) {
        frontend::ast::PrettyPrinter{os}.visit(decl);
        os << '\n';
    }
    for (auto& [name, decl] : tu.functions) {
        frontend::ast::PrettyPrinter{os}.visit(*decl);
        os << '\n';
    }
}
}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println(stderr, "too few arguments");
        return EXIT_FAILURE;
    }

    std::filesystem::path filename;

    std::string input;
    if (argv[1] != "-"sv) {
        filename = argv[1];
        filename = filename.filename();

        std::ifstream file{argv[1]};
        if (!file.is_open()) {
            std::println(stderr, "could not find file {}", argv[1]);
            return EXIT_FAILURE;
        }
        input = std::string{std::istreambuf_iterator{file}, {}};
    } else {
        filename = "stdin";
        input = std::string{std::istreambuf_iterator{std::cin}, {}};
    }

    bool dumpAst = false;
    bool dumpIr = false;
    bool dumpLir = false;
    for (int i = 2; i < argc; ++i) {
        if (argv[i] == "--dump-ast"sv) {
            dumpAst = true;
        } else if (argv[i] == "--dump-ir"sv) {
            dumpIr = true;
        } else if (argv[i] == "--dump-lir"sv) {
            dumpLir = true;
        } else {
            std::println(stderr, "unknown option {}", argv[i]);
            return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }

    frontend::TranslationUnit tu;
    try {
        tu = frontend::runSema(std::move(file));
    } catch (const frontend::Error& e) {
        std::print(stderr, "error: {}", e.pretty());
        return EXIT_FAILURE;
    }

    if (dumpAst) {
        std::ofstream dump{std::string{filename} + ".ast.dump"};
        dumpTranslationUnit(dump, tu);
    }

    ir::Context ctx;
    ir::Module irMod = frontend::ast2ir::translate(ctx, tu);

    if (dumpIr) {
        std::ofstream dump{std::string{filename} + ".ir.dump"};
        ir::PrettyPrinter irPretty{dump};
        irPretty.visit(irMod);
    }

    lir::Module lirMod = backend::lowerIr(irMod);

    if (dumpLir) {
        std::ofstream dump{std::string{filename} + ".lir.dump"};
        lir::PrettyPrinter lirPretty{dump};
        lirPretty.visit(lirMod);
    }
}
