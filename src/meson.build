project('openpower-hw-diags', 'cpp',
        version: '0.1', meson_version: '>=0.49.0',
        default_options: [
          'warning_level=3',
          'werror=true',
          'cpp_std=c++17'
        ])

#subdir('openpower-hw-diags-sim')
#subdir('openpower-hw-diags-test')

executable('openpower-hw-diags', 'main.cpp', install : true)

build_simulator = get_option('build-simulator')
build_tests = get_option('build-tests')

if build_simulator
    subdir('sim')
endif

if build_tests
    subdir('test')
endif
