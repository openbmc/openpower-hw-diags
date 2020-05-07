# gather analyzer sources to be used here and elsewhere if needed
analyzer_src = files('analyzer_main.cpp')

hei_user_src = files('hei_user_defines.cpp',
                     'hei_user_interface.cpp')

# Create hardware error analyzer library
analyzer = static_library('analyzer',
                          analyzer_src, hei_user_src,
                          dependencies : libhei_dep,
                          install : false)
