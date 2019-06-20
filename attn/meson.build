pdbg = meson.get_compiler('cpp').find_library('pdbg')

sdbusplus = dependency('sdbusplus')

executable('attn',
           'attn_main.cpp', 'attn_handler.cpp',
           dependencies : [pdbg, sdbusplus],
           install : true)
