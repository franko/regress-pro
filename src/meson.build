gsl_dep = dependency('gsl')
thread_dep = dependency('threads')
nlopt_dep = dependency('nlopt')
kernel_incdir = include_directories('.')

configure_file(output : 'defs.h', configuration : conf)

kernel_src = [
    'common.cpp', 'data-table.cpp', 'data-view.cpp', 'rc_matrix.cpp', 'disp-table.cpp', 'disp-sample-table.cpp',
    'disp-lookup.cpp', 'str.cpp', 'dispers-library.cpp', 'str-util.cpp', 'error-messages.cpp',
    'minsampling.cpp', 'dispers.cpp', 'disp-fb.cpp', 'disp-tauc-lorentz.cpp', 'disp-ho.cpp', 'disp-lorentz.cpp',
    'disp-bruggeman.cpp', 'disp-cauchy.cpp', 'disp-sellmeier.cpp', 'dispers-classes.cpp', 'disp-deriv-check.cpp', 'stack.cpp', 'lmfit.cpp',
    'fit-timestamp.cpp', 'lmfit-simple.cpp', 'fit-params.cpp', 'lmfit-mlr-adapter.cpp', 'fit-engine.cpp', 'refl-kernel.cpp',
    'number-parse.cpp', 'refl-utils.cpp', 'acquisition.cpp', 'gauss-legendre-quad.cpp', 'spectra.cpp',
    'elliss.cpp', 'multi-fit-engine.cpp', 'nlopt_fit.cpp', 'lmfit-multi.cpp', 'disp-fit-engine.cpp', 'vector_print.cpp',
    'fit_result.cpp', 'writer.cpp', 'lexer.cpp', 'lmfit_result.cpp', 'disp-load-binary.cpp']

libkernel = static_library('regpro-kernel', kernel_src, dependencies: [gsl_dep, thread_dep, nlopt_dep])
