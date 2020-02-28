libhei_dep = dependency('libhei', fallback : ['libhei', 'libhei_dep'])

executable('openpower-hw-diags-test',
            'main.cpp', 'logging.cpp', 'bp_handler.cpp', 'ti_handler.cpp',
            'analyzer_main.cpp',
            link_with : [analyzer, attn],
            include_directories : incdir,
            dependencies : libhei_dep,
            install : false)
