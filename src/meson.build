sdbusplus = dependency('sdbusplus')

# add source root in the includes search path (for org/open_power/...)
incdir = include_directories('..')

# create openpower-hw-diags application
executable('openpower-hw-diags',
           'main.cpp',
           '../org/open_power/HwDiags/server.cpp',
           '../org/open_power/HwDiags/error.cpp',
           dependencies : sdbusplus,
           include_directories : incdir,
           install : true)
