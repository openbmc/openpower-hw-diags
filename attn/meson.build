pdbg = meson.get_compiler('cpp').find_library('pdbg')

executable('attn',
           'attn_main.cpp', 'attn_handler.cpp',
           dependencies : pdbg,
           install : true)
