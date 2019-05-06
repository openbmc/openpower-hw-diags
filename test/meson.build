tests = [
  'hello-world',
  'bitstring',
]

foreach t : tests
  test(t, executable(t.underscorify(), t + '.cpp', include_directories : incdir))
endforeach

gtest = dependency('gtest', main : true, required : false, method : 'system')

if gtest.found()
    e = executable('bitstring_gtest', 'bitstring_gtest.cpp', \
                    dependencies : gtest, \
                    include_directories : incdir)

test('bitstring_gtest', e)
endif
