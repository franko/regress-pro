project('regress-pro', 'c', 'cpp', version: '2.0.2', default_options : ['cpp_std=c++14'])

if host_machine.system() == 'windows'
    add_global_arguments('-DWIN32', language : ['c', 'cpp'])
endif

subdir('src')
subdir('fox-gui')
