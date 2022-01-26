# Get the gtest/gmock dependencies.
gtest_dep = dependency('gtest', main: true, disabler: true, required: false)
gmock_dep = dependency('gmock', disabler: true, required: false)
if not gtest_dep.found() or not gmock_dep.found()
    cmake = import('cmake')
    gtest_proj = cmake.subproject('googletest',
                                  required: false)
    if gtest_proj.found()
        gtest_dep = declare_dependency(
            dependencies: [
                dependency('threads'),
                gtest_proj.dependency('gtest'),
                gtest_proj.dependency('gtest_main'),
            ]
        )
        gmock_dep = gtest_proj.dependency('gmock')
    else
        assert(not get_option('tests').enabled(),
               'Googletest is required if tests are enabled')
    endif
endif

# NOTE: We cannot link to `util_lib` because that is built without
#       `-DTEST_TRACE` and any of the util functions that use `trace.hpp` will
#       throw a linker error because we don't have access to phosphor-logging
#       in test ... yet.

test_arg = [
    '-DTEST_TRACE',
]

# Compile the test dts into a binary for pdbg.
pdbg_test_dtb = custom_target('build_pdbg_test_dtb',
    input   : files('pdbg_test.dts'),
    output  : 'pdbg_test.dtb',
    command : [ find_program('dtc'), '-I', 'dts', '-O', 'dtb',
                '-o', '@OUTPUT@', '@INPUT@' ])

pdbg_env = 'PDBG_DTB=' + pdbg_test_dtb.full_path()

# end2end code exerciser for experiment and testing
subdir('end2end')

################################################################################

tc = 'bin_stream_test'

src = [
  files(
    tc + '.cpp',
  ),
]

dep = [ gtest_dep ]

var = [ ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 cpp_args: test_arg, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'ffdc_file_test'

src = [
  files(
    tc + '.cpp',
    '../util/ffdc_file.cpp',
    '../util/temporary_file.cpp',
  ),
]

dep = [ gtest_dep ]

var = [ ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 cpp_args: test_arg, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'resolution_test'

src = [
  files(
    tc + '.cpp',
    '../analyzer/resolution.cpp',
    '../analyzer/service_data.cpp',
    '../util/pdbg.cpp',
  ),
  pdbg_test_dtb,
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 cpp_args: test_arg, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'pdbg_dts_test'

src = [
  files(
    tc + '.cpp',
    '../util/pdbg.cpp',
  ),
  pdbg_test_dtb,
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 cpp_args: test_arg, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-lpc-timeout'

src = [
  files(
    tc + '.cpp',
    '../analyzer/plugins/p10-plugins.cpp',
    '../analyzer/service_data.cpp',
    '../util/pdbg.cpp',
    'pdbg-sim-only.cpp',
  ),
  pdbg_test_dtb,
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 cpp_args: test_arg, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-pll-unlock'

src = [
  files(
    tc + '.cpp',
    '../analyzer/plugins/p10-plugins.cpp',
    '../analyzer/service_data.cpp',
    '../util/pdbg.cpp',
    'pdbg-sim-only.cpp',
  ),
  pdbg_test_dtb,
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 cpp_args: test_arg, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-tod-step-check-fault'

src = [
  files(
    tc + '.cpp',
    '../analyzer/plugins/p10-tod-plugins.cpp',
    '../analyzer/service_data.cpp',
    '../util/pdbg.cpp',
  ),
  pdbg_test_dtb,
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 cpp_args: test_arg, include_directories: incdir)

test(tc, exe, env: var)

