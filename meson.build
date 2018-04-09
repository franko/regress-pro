project('regress-pro', 'cpp', version: '2.1.0', default_options : ['cpp_std=c++14'])

debug_build = get_option('buildtype') != 'release'
dev_tag = get_option('dev_tag')

conf = configuration_data()
conf.set('VERSION_MAJOR', '2')
conf.set('VERSION_MINOR', '1')
conf.set('VERSION_PATCH', '0')
conf.set_quoted('VERSION_BUILD_EXT', dev_tag)
conf.set('RELEASE_BUILD', get_option('release_build'))
conf.set_quoted('HOST_SYSTEM', host_machine.system())
conf.set_quoted('CPU_FAMILY', host_machine.cpu_family())

if host_machine.system() == 'windows'
    add_global_arguments('-DWIN32', language : 'cpp')
endif

if meson.get_compiler('cpp').get_id() == 'gcc'
    add_global_arguments('-ffast-math', language : 'cpp')
    if not debug_build
        add_global_arguments(['-fno-exceptions', '-fno-rtti'], language : 'cpp')
    endif
endif

if debug_build
    add_global_arguments('-DDEBUG', '-DDEBUG_MEM', language : 'cpp')
endif

subdir('src')
subdir('fox-gui')
