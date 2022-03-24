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

################################################################################

# Compile the test dts into a binary for pdbg.
pdbg_test_dtb = custom_target('build_pdbg_test_dtb',
    input   : files('pdbg-test.dts'),
    output  : 'pdbg-test.dtb',
    command : [ find_program('dtc'), '-I', 'dts', '-O', 'dtb',
                '-o', '@OUTPUT@', '@INPUT@' ])

pdbg_env = 'PDBG_DTB=' + pdbg_test_dtb.full_path()

################################################################################

# IMPORTANT NOTE:
# We cannot link to `util_lib` because:
#   - It is built without `-DTEST_TRACE` and any of the util functions that use
#     `trace.hpp` will throw a linker error because we don't have access to
#     phosphor-logging in test ... yet.
#   - Some functions related to pdbg and dbus simply cannot be built in the test
#     environment. Instead, there are alternate implementation of those
#     functions to simulate them for testing.

test_args = [
  '-DTEST_TRACE',
  package_args,
]

test_util_srcs = [
  files(
    '../util/data_file.cpp',
    '../util/ffdc_file.cpp',
    '../util/pdbg.cpp',
    '../util/temporary_file.cpp',
    'dbus-sim-only.cpp',
    'pdbg-sim-only.cpp',
  ),
  pdbg_test_dtb
]

test_deps = [
  libhei_dep,
  libpdbg_dep,
  gtest_dep,
]

test_vars = [
  pdbg_env,
]

test_util_lib = static_library('test_util_lib',
  sources             : test_util_srcs,
  include_directories : incdir,
  dependencies        : test_deps,
  cpp_args            : test_args,
  install             : false,
)

test_libs = [
  analyzer_lib,
  attn_lib,
  test_util_lib,
]

################################################################################

tc = 'test-bin-stream'

src = [
  files(
    tc + '.cpp',
  ),
]

dep = [ gtest_dep ]

var = [ ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with: test_libs,
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-ffdc-file'

src = [
  files(
    tc + '.cpp',
  ),
]

dep = [ gtest_dep ]

var = [ ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with: test_libs,
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-lpc-timeout'

src = [
  files(
    tc + '.cpp',
    '../analyzer/plugins/p10-plugins.cpp',
  ),
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with: test_libs,
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-pdbg-dts'

src = [
  files(
    tc + '.cpp',
  ),
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with: test_libs,
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-pll-unlock'

src = [
  files(
    tc + '.cpp',
    '../analyzer/plugins/p10-plugins.cpp',
  ),
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with: test_libs,
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-resolution'

src = [
  files(
    tc + '.cpp',
  ),
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with: test_libs,
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-tod-step-check-fault'

src = [
  files(
    tc + '.cpp',
    '../analyzer/plugins/p10-tod-plugins.cpp',
  ),
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with: test_libs,
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-ti-handler'

src = [
  files(
    tc + '.cpp',
  ),
]

dep = [ gtest_dep ]

var = [ ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with : hwdiags_libs, # TODO: should use test_libs instead
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

################################################################################

tc = 'test-attention'

src = [
  files(
    tc + '.cpp',
  ),
  pdbg_test_dtb,
]

dep = [ gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with : hwdiags_libs, # TODO: should use test_libs instead
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)


################################################################################

tc = 'test-end2end'

src = [
  files(
    tc + '.cpp',
    '../cli.cpp',
  ),
  pdbg_test_dtb,
]

dep = [ gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 link_with : hwdiags_libs, # TODO: should use test_libs instead
                 cpp_args: test_args, include_directories: incdir)

test(tc, exe, env: var)

