sdbusplus = dependency('sdbusplus')

# generate sdbus bindings for openpower-hw-diags application
run_command(find_program('generate'), 'org.open_power.hw.Diags', 'org/open_power/hw/Diags')

# create openpower-hw-diags application
executable('openpower-hw-diags',
           'main.cpp', 'org/open_power/hw/Diags/server.cpp', 'org/open_power/hw/Diags/error.cpp',
           dependencies : sdbusplus,
           install : true)
