#include <fstream>
#include <iostream>
#include <print>

#include "frontend/lex/lexer.hpp"
#include "frontend/parse/parser.hpp"

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

    frontend::TranslationUnit tu;
    try {
        tu = parser.parse();
    } catch (const frontend::Error& e) {
        std::println(stderr, "syntax error: {}", e.pretty());
    }

    bool first = true;
    for (const auto& [name, decl] : tu.decls) {
        if (!first) {
            std::cout << '\n';
        }
        first = false;
        decl->pretty(std::cout);
    }
}
