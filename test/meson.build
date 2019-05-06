tests = [
  'hello-world',
  'bitstring',
]

foreach t : tests
  test(t, executable(t.underscorify(), t + '.cpp', include_directories : incdir))
endforeach
