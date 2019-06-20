# dependency to link dbus support
sdbusplus = dependency('sdbusplus')

# needed to find external libraries
cmplr = meson.get_compiler('cpp')

# link in external libpdbg support (requires libfdt also)
lib1 = cmplr.find_library('pdbg')
lib2 = cmplr.find_library('fdt')

# statically link local libpdbg support
#dir = join_paths(meson.current_source_dir(), 'libs')
#lib1 = cmplr.find_library('xyzpdbg', dirs : dir)
#lib2 = cmplr.find_library('xyzfdt' , dirs : dir)

dep_lib1 = declare_dependency(link_args : '-Wl,--whole-archive')
dep_lib2 = declare_dependency(link_args : '-Wl,--no-whole-archive')

executable('attn',
           'attn_main.cpp', 'attn_handler.cpp',
           dependencies : [dep_lib1, lib1, dep_lib2, lib2, sdbusplus],
           install : true)
