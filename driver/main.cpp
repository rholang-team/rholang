#include <fstream>
#include <iostream>
#include <print>

#include "frontend/ast/prettyprint.hpp"
#include "frontend/lex/lexer.hpp"
#include "frontend/parse/parser.hpp"
#include "frontend/sema.hpp"

namespace {
void printAst(frontend::ast::File& file) {
    for (auto& [name, decl] : file.structs) {
        frontend::ast::PrettyPrinter{std::cout}.visit(decl);
        std::cout << '\n';
    }
    for (auto& [name, decl] : file.globals) {
        frontend::ast::PrettyPrinter{std::cout}.visit(decl);
        std::cout << '\n';
    }
    for (auto& [name, decl] : file.functions) {
        frontend::ast::PrettyPrinter{std::cout}.visit(decl);
        std::cout << '\n';
    }
}

void printTranslationUnit(frontend::TranslationUnit& tu) {
    for (auto& [name, s] : tu.structs) {
        std::println("{}\n", *s);
    }
    for (auto& [name, decl] : tu.globals) {
        frontend::ast::PrettyPrinter{std::cout}.visit(decl);
        std::cout << '\n';
    }
    for (auto& [name, decl] : tu.functions) {
        frontend::ast::PrettyPrinter{std::cout}.visit(decl);
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

    frontend::lex::Lexer lexer{std::move(input)};
    frontend::lex::Lexemes lexemes{lexer.lex()};
    frontend::parse::Parser parser{std::move(lexemes)};

    frontend::ast::File file;
    try {
        file = parser.parse();
    } catch (const frontend::Error& e) {
        std::println(stderr, "syntax error: {}", e.pretty());
    }

    printAst(file);

    frontend::TranslationUnit tu;
    try {
        tu = frontend::runSema(std::move(file));
    } catch (const frontend::Error& e) {
        std::cout << e.pretty() << '\n';
    }

    printTranslationUnit(tu);
}
