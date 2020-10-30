# gather analyzer sources to be used here and elsewhere if needed
analyzer_src = files(
    'analyzer_main.cpp',
    'hei_user_interface.cpp',
    '../util/ffdc_file.cpp',
    '../util/temporary_file.cpp',
)

# Create hardware error analyzer library
analyzer = static_library('analyzer',
                          analyzer_src,
                          include_directories : incdir,
                          dependencies : [ libhei_dep, sdbusplus_dep ],
                          cpp_args : test_arg,
                          install : false)
