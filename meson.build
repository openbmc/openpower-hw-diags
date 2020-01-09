project('openpower-hw-diags', 'cpp',
        version: '0.1', meson_version: '>=0.49.0',
        default_options: [
          'warning_level=3',
          'werror=true',
          'cpp_std=c++17'
        ])

# generate sdbus bindings for openpower-hw-diags application
run_command(find_program('generate', required : true),
                         'org.open_power.HwDiags',
                         'org/open_power/HwDiags')

subdir('src')
subdir('attn')

build_tests = get_option('tests')

if not build_tests.disabled()
  subdir('test')
endif
