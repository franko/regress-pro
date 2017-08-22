project('regress-pro', 'cpp', version: '2.0.2', default_options : ['cpp_std=c++14'])

conf = configuration_data()
conf.set('VERSION_MAJOR', '2')
conf.set('VERSION_MINOR', '0')
conf.set('VERSION_PATCH', '2')
conf.set('RELEASE_BUILD', get_option('release_build'))
conf.set_quoted('HOST_SYSTEM', host_machine.system())
conf.set_quoted('CPU_FAMILY', host_machine.cpu_family())

enable_debug = get_option('buildtype') != 'release'

if host_machine.system() == 'windows'
    add_global_arguments('-DWIN32', language : 'cpp')
endif

if enable_debug
    add_global_arguments(['-DDEBUG', '-DDEBUG_MEM'], language : 'cpp')
endif

if meson.get_compiler('cpp').get_id() == 'gcc'
    add_global_arguments(['-ffast-math'], language : 'cpp')
    if not enable_debug
        add_global_arguments(['-fno-exceptions', '-fno-rtti'], language : 'cpp')
    endif
endif

subdir('src')
subdir('fox-gui')
