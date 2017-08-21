project('regress-pro', 'c', 'cpp', version: '2.0.2', default_options : ['cpp_std=c++14'])

debug_build = get_option('buildtype') != 'release'

if host_machine.system() == 'windows'
    add_global_arguments('-DWIN32', language : ['c', 'cpp'])
endif

if meson.get_compiler('cpp').get_id() == 'gcc'
    add_global_arguments('-ffast-math', language : ['c', 'cpp'])
    if not debug_build
        add_global_arguments(['-fno-exceptions', '-fno-rtti'], language : 'cpp')
        add_global_arguments(['-fomit-frame-pointer'], language : 'c')
    endif
endif

if debug_build
    add_global_arguments('-DDEBUG', '-DDEBUG_MEM', language : ['c', 'cpp'])
endif

subdir('src')
subdir('fox-gui')
