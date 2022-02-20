project('regress-pro', ['c', 'cpp'], version: '2.0.2', default_options : ['c_std=c99', 'cpp_std=c++11'])

debug_build = get_option('buildtype') != 'release'

regpro_cargs = ['-DHAVE_INLINE']
regpro_core_args = []
if debug_build
    regpro_core_args += ['-DDEBUG_MEM', '-DDEBUG_REGRESS']
endif

if host_machine.system() == 'windows'
    regpro_core_args += '-DWIN32'
endif

if get_option('registration')
    regpro_core_args += '-DREGPRO_REGISTRATION'
endif

regpro_link_args = []
compiler_id = meson.get_compiler('cpp').get_id()
if not debug_build and compiler_id == 'gcc'
    regpro_link_args += ['-static-libgcc', '-static-libstdc++']
endif

subdir('src')
subdir('fox-gui')

