project('regress-pro', 'cpp', version: '2.0.2', default_options : ['cpp_std=c++14'])

enable_debug = get_option('debug')

if host_machine.system() == 'windows'
    add_global_arguments('-DWIN32', language : ['cpp'])
endif

if enable_debug
    add_global_arguments('-DDEBUG -DDEBUG_MEM', language : ['cpp'])
endif

subdir('src')
subdir('fox-gui')
