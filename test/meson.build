tests = [
  'hello-world',
  'bitstring',
]

foreach t : tests
  test(t, executable(t.underscorify(), t + '.cpp', include_directories : incdir))
endforeach

build_gtests = get_option('gtests')

if not build_gtests.disabled()
  gtest_proj = subproject('gtest')
  gtest = gtest_proj.get_variable('gtest_main_dep')

  e = executable('openpower-hw-diags-gtest', 'openpower-hw-diags-gtest.cpp', \
                  dependencies : gtest, \
                  include_directories : incdir)

  test('gtest openpower-hw-diags-test', e)
endif
