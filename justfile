builddir := "./build"

run: build
    {{builddir}}/compiler examples/foo

test:
    meson test -C {{builddir}}
    
build:
    meson compile -C {{builddir}}

setup_debug:
    meson setup {{builddir}} --buildtype=debug -Db_lundef=false -Db_sanitize=address,undefined --reconfigure

setup_release:
    meson setup {{builddir}} --buildtype=release --reconfigure
