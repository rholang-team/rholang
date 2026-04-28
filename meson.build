project(
    'rholang',
    'cpp',
    default_options: [
        'cpp_std=c++23',
        'warning_level=3',
    ],
)
add_project_arguments('-Wno-c99-extensions', language: 'cpp')

gtest_dep = dependency('gtest_main')

runtime_src = [
    'runtime/main_alloc.cpp',
    'runtime/runtime.cpp',
    'runtime/bin.cpp',
    'runtime/fla.cpp',
]

runtime_tests_src = [
    'runtime/test/bin_test.cpp',
    'runtime/test/fla_test.cpp',
    'runtime/test/main_alloc_test.cpp',
    'runtime/test/runtime_test.cpp',
]

runtime = static_library('runtime', runtime_src)
test(
    'runtime tests',
    executable(
        'runtimetests',
        runtime_tests_src,
        dependencies: [gtest_dep],
        link_with: runtime,
    ),
    suite: 'runtime'
)

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
    'compiler/ir/instr.cpp',
    'compiler/ir/builder.cpp',
    'compiler/ir/bb.cpp',
    'compiler/ir/value.cpp',
    'compiler/ir/module.cpp',
    'compiler/ir/type.cpp',
    'compiler/ir/prettyprint.cpp',
]

lir_src = [
    'compiler/lir/bb.cpp',
    'compiler/lir/function.cpp',
    'compiler/lir/instr.cpp',
    'compiler/lir/module.cpp',
    'compiler/lir/prettyprint.cpp',
    'compiler/lir/register.cpp',
]

backend_src = [
    'compiler/backend/codegen.cpp',
    'compiler/backend/lowerir.cpp',
    'compiler/backend/interferencegraph.cpp',
]

compiler_src = frontend_src + ir_src + lir_src + backend_src

compiler_tests_src = [
    'compiler/tests/frontend.cpp',
    'compiler/tests/translator.cpp',
]

compiler = static_library('compiler', compiler_src)

test(
    'compiler tests',
    executable(
        'compilertests',
        compiler_tests_src,
        dependencies: [gtest_dep],
        link_with: [compiler],
    ),
    suite: 'compiler'
)

driver_src = [
    'driver/main.cpp',
]
executable('compiler', driver_src, link_with: [compiler])
