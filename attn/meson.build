# needed to find external libraries not registered with package manager
cmplr = meson.get_compiler('cpp')

# async gpio monitor needs boost library
boost_args = ['-DBOOST_ASIO_DISABLE_THREADS',
              '-DBOOST_ERROR_CODE_HEADER_ONLY',
              '-DBOOST_SYSTEM_NO_DEPRECATED']

# get sdbusplus dependency if building with dbus support enabled
if not dbus_support.disabled()
    sdbusplus = dependency('sdbusplus', version : '>=1.0', required : true)
else
    sdbusplus = declare_dependency()
endif

# dependency to link gpiod support
libgpiod = dependency('libgpiod', version : '>=1.4.1')

# dependency to link libpdbg support
libpdbg = cmplr.find_library('pdbg')

# libpdbg requires linking with "whole-archive" option
whole_archive = declare_dependency(link_args : '-Wl,--whole-archive')
no_whole_archive = declare_dependency(link_args : '-Wl,--no-whole-archive')

# install systemd unit file
configure_file(
    input: 'attn_handler.service',
    output: 'attn_handler.service',
    copy: true,
    install_dir:
        dependency('systemd').get_pkgconfig_variable(
            'systemdsystemunitdir')
)

# gather attention sources to be used here and elsewhere if needed
attn_src = files('attn_main.cpp', 'attn_handler.cpp', 'attn_monitor.cpp')

# Create attention handler library
attn = static_library('attn_handler',
                      attn_src, bp_handler_src, logging_src, ti_handler_src,
                      include_directories : incdir,
                      dependencies : [whole_archive, libpdbg,
                                      no_whole_archive, sdbusplus, libgpiod],
                      cpp_args : boost_args,
                      install : true)
