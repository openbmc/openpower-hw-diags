# Source files.
analyzer_src = files(
    'analyzer_main.cpp',
    'create_pel.cpp',
    'filter-root-cause.cpp',
    'hei_user_interface.cpp',
    'initialize_isolator.cpp',
    'ras-data/ras-data-parser.cpp',
    'resolution.cpp',
    'service_data.cpp',
)

plugins_src = files(
    'plugins/ody-plugins.cpp',
    'plugins/p10-plugins.cpp',
    'plugins/p10-tod-plugins.cpp',
)

# Library dependencies.
analyzer_deps = [
    dbus_interfaces_dep,
    libhei_dep,
    nlohmann_json_dep,
    phosphor_logging_dep,
    sdbusplus_dep,
    valijson_dep,
]

# Create static library.
analyzer_lib = static_library(
    'analyzer_lib',
    [analyzer_src],
    include_directories: incdir,
    dependencies: analyzer_deps,
    cpp_args: [package_args],
    install: false,
)

# Install the RAS data files.
subdir('ras-data')

