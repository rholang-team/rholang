builddir := "./build"

run FILE: build
    {{builddir}}/compiler {{FILE}}

test:
    meson test -C {{builddir}}

format:
    clang-format -i $(find . -name "**.hpp") $(find . -name "**.cpp")
    
build:
    meson compile -C {{builddir}}

setup_debug:
    meson setup {{builddir}} --buildtype=debug -Db_lundef=false -Db_sanitize=address,undefined --reconfigure

setup_release:
    meson setup {{builddir}} --buildtype=release --reconfigure
