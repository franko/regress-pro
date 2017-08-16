gsl_dep = dependency('gsl')
thread_dep = dependency('threads')
kernel_incdir = include_directories('.')
kernel_src = ['common.c', 'data-table.c', 'data-view.c', 'rc_matrix.c', 'disp-table.c', 'disp-sample-table.c', 'disp-lookup.c', 'str.c', 'dispers-library.c', 'str-util.c', 'batch.c', 'error-messages.c', 'cmpl.c', 'minsampling.c', 'dispers.c', 'disp-fb.c', 'disp-tauc-lorentz.c', 'disp-ho.c', 'disp-lorentz.c', 'disp-bruggeman.c', 'disp-cauchy.c', 'disp-sellmeier.c', 'dispers-classes.c', 'stack.c', 'lmfit.c', 'fit-timestamp.c', 'lmfit-simple.c', 'fit-params.c', 'fit-engine.c', 'refl-kernel.c', 'refl-fit.c', 'elliss-fit.c', 'number-parse.c', 'refl-utils.c', 'acquisition.c', 'gauss-legendre-quad.c', 'spectra.c', 'elliss.c', 'multi-fit-engine.c', 'grid-search.c', 'lmfit-multi.c', 'disp-fit-engine.c', 'vector_print.c', 'fit_result.c', 'writer.c', 'lexer.c', 'lmfit_result.c']
libkernel = static_library('regpro-kernel', kernel_src, dependencies: [gsl_dep, thread_dep])
