builddir := "build"

run *ARGS: build
    {{builddir}}/compiler {{ARGS}}

test *ARGS:
    meson test -C {{builddir}} {{ARGS}}

test-compiler:
    meson test -C {{builddir}} --suite compiler

test-runtime:
    meson test -C {{builddir}} --suite runtime

[working-directory: 'compiler-fuzzing']
fuzz: build
    sbt "run '../{{builddir}}/compiler'"

format:
    clang-format -i $(find . -name "**.hpp") $(find . -name "**.cpp")
    
build:
    meson compile -C {{builddir}}

setup_debug:
    meson setup {{builddir}} --buildtype=debug -Db_lundef=false -Db_sanitize=address,undefined --reconfigure

setup_release:
    meson setup {{builddir}} --buildtype=release --reconfigure
