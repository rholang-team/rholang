project(
    'language',
    'cpp',
    default_options: [
        'cpp_std=c++23',
        'warning_level=3',
    ],
)

gtest_dep = dependency('gtest_main')

runtime_src = [
    # put your paths here
]

runtime = static_library('runtime', runtime_src)

frontend_src = [
    'compiler/frontend/type.cpp',
    'compiler/frontend/error.cpp',
    'compiler/frontend/lex/lexeme.cpp',
    'compiler/frontend/lex/lexer.cpp',
    'compiler/frontend/parse/parser.cpp',
    'compiler/frontend/ast/prettyprint.cpp',
    'compiler/frontend/ast/decl.cpp',
    'compiler/frontend/ast/expr.cpp',
    'compiler/frontend/sema.cpp',
    'compiler/frontend/mangling.cpp',
    'compiler/frontend/ast2ir.cpp',
]

ir_src = [
    'compiler/ir/typesets.cpp',
    'compiler/ir/context.cpp',
    'compiler/ir/value.cpp',
    'compiler/ir/type.cpp',
    'compiler/ir/instr.cpp',
    'compiler/ir/builder.cpp',
]

backend_src = []

compiler_src = frontend_src + ir_src + backend_src
compiler = static_library('compiler', compiler_src)

driver_src = [
    'driver/main.cpp',
]
executable('compiler', driver_src, link_with: [compiler])
