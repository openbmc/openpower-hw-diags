libhei_dep = dependency('libhei', fallback : ['libhei', 'libhei_dep'])

executable('openpower-hw-diags-test', 'main.cpp',
            link_with : [analyzer, attn],
            include_directories : incdir,
            dependencies : libhei_dep,
            install : false)
