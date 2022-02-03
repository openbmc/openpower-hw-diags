# Source files.
util_src = files(
    'data_file.cpp',
    'dbus.cpp',
    'ffdc_file.cpp',
    'pdbg.cpp',
    'pdbg-no-sim.cpp',
    'temporary_file.cpp',
)

# Library dependencies.
util_deps = [
    libhei_dep,
    libpdbg_dep,
]

if get_option('phal').enabled()
    util_deps += dependency('libdt-api')
endif

# Compiler arguments.
util_args = [
]

