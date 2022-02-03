# Source files.
analyzer_src = files(
    'analyzer_main.cpp',
    'create_pel.cpp',
    'filter-root-cause.cpp',
    'hei_user_interface.cpp',
    'initialize_isolator.cpp',
    'resolution.cpp',
    'service_data.cpp',
)

# Library dependencies.
analyzer_deps = [
    dbus_interfaces_dep,
    libhei_dep,
    nlohmann_json_dep,
    sdbusplus_dep,
    valijson_dep,
]

# Compiler arguments.
analyzer_args = [
    package_args,
]

# Search subdirectories for additional sources, dependencies, etc.
subdir('plugins')
subdir('ras-data') # also installs the RAS data files

