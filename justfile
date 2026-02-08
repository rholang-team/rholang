builddir := "./build"

run: compile
    {{builddir}}/compiler examples/foo
    
compile:
    ninja -C {{builddir}}

setup_debug:
    meson setup {{builddir}} --buildtype=debug -Db_lundef=false -Db_sanitize=address,undefined --reconfigure

setup_release:
    meson setup {{builddir}} --buildtype=release --reconfigure
