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

# Add gtest/gmock dependency to the list of test dependencies.
test_deps = [
  test_hwdiags_deps,
  gtest_dep,
]

# Add pdbg_test_dtb to pdbg test environment.
pdbg_env = 'PDBG_DTB=' + pdbg_test_dtb.full_path()

test_vars = [
  pdbg_env,
  'LG2_FORCE_STDERR=true',
]

# Additional SRCs that are not (or should not be) included in libraries.
# NOTE: Try to limit this, if possible, to prevent duplicate compilation.
test_additional_srcs = [
  '../analyzer/plugins/p10-plugins.cpp',
  '../analyzer/plugins/p10-tod-plugins.cpp',
  '../cli.cpp',
]

################################################################################

testcases = [
  'test-bin-stream',
  'test-ffdc-file',
  'test-lpc-timeout',
  'test-pdbg-dts',
  'test-pll-unlock',
  'test-resolution',
  'test-tod-step-check-fault',
]

foreach tc : testcases

  exe = executable(tc.underscorify(),
    sources             : [ files(tc + '.cpp'), test_additional_srcs ],
    include_directories : incdir,
    dependencies        : test_deps,
    cpp_args            : test_args,
    link_with           : test_libs,
  )

  test(tc, exe, env: test_vars)

endforeach

################################################################################

testcases = [
  'test-attention',
  'test-end2end',
  'test-util-data-file',
  'test-ti-handler',
]

foreach tc : testcases

  exe = executable(tc.underscorify(),
    sources             : [ files(tc + '.cpp'), test_additional_srcs ],
    include_directories : incdir,
    dependencies        : test_deps,
    cpp_args            : test_args,
    link_with           : hwdiags_libs,
  )

  test(tc, exe, env: test_vars)

endforeach

