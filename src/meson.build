sdbusplus = dependency('sdbusplus')

# add source root in the includes search path (for org/open_power/...)
incdir = include_directories('..')

# If libhei is not proivided by the system then build using the subproject
libhei_dep = dependency('libhei', fallback : ['libhei', 'libhei_dep'])

# create openpower-hw-diags application
executable('openpower-hw-diags',
           'main.cpp',
           'user_interface.cpp',
           '../org/open_power/HwDiags/server.cpp',
           '../org/open_power/HwDiags/error.cpp',
           dependencies : [sdbusplus, libhei_dep],
           include_directories : incdir,
           install : true)
