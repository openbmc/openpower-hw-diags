# dependency to link dbus support
sdbusplus = dependency('sdbusplus')

# needed to find external libraries
cmplr = meson.get_compiler('cpp')
libpdbg = cmplr.find_library('pdbg')

# create attention handler application
executable('attn_handler',
           'attn_main.cpp', 'attn_handler.cpp', 'spattn.cpp',
           dependencies : [libpdbg, sdbusplus],
           install : true)

# Uncomment the code below to statically link the pdbg library. Statically
# linking pdbg also requires the fdt library. The build will look for
# libpdbg.a and libfdt.a in the attn/libs directory.

## statically link local libpdbg support
#dir = join_paths(meson.current_source_dir(), 'libs')
#lib1 = cmplr.find_library('pdbg', dirs : dir)
#lib2 = cmplr.find_library('fdt' , dirs : dir)

## pdbg library requires whole-archive linking, fdt library does not
#dep_lib1 = declare_dependency(link_args : '-Wl,--whole-archive')
#dep_lib2 = declare_dependency(link_args : '-Wl,--no-whole-archive')

## create attention handler application (statically linked)
#executable(app_name+'_static',
#           'attn_main.cpp', 'attn_handler.cpp', spattn_handler,
#           dependencies : [dep_lib1, lib1, dep_lib2, lib2, sdbusplus],
#           install : true)
