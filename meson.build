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
    'frontend/type.cpp',
    'frontend/lex/lexeme.cpp',
    'frontend/lex/lexer.cpp',
    'frontend/parse/parser.cpp',
    'frontend/parse/error.cpp',
    'frontend/pretty.cpp',
]
frontend = static_library('frontend', frontend_src)
test(
    'frontend tests',
    executable(
        'frontendtests',
        [],
        dependencies: [gtest_dep],
    ),
)

driver_src = [
    'driver/main.cpp',
]
executable('compiler', driver_src, link_with: [frontend])
