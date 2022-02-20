regpro_core_deps = []
regpro_core_deps += dependency('gsl')
regpro_core_deps += dependency('threads')
libell_include = include_directories('.')

libell_src = [
    'common.c', 'data-table.c', 'data-view.c', 'rc_matrix.c', 'disp-table.c',
	'disp-sample-table.c', 'disp-lookup.c', 'str.c', 'dispers-library.c', 'str-util.c',
	'batch.c', 'error-messages.c', 'cmpl.c', 'minsampling.c', 'dispers.c', 'disp-fb.c',
	'disp-tauc-lorentz.c', 'disp-ho.c',
	'disp-bruggeman.c', 'disp-cauchy.c', 'dispers-classes.c', 'stack.c', 'lmfit.c',
	'lmfit-simple.c', 'fit-params.c', 'fit-engine.c', 'refl-kernel.c',
	'refl-fit.c', 'elliss-fit.c', 'number-parse.c', 'refl-utils.c', 'spectra.c', 'elliss.c', 'test-deriv.c',
	'elliss-multifit.c', 'multi-fit-engine.c', 'grid-search.c', 'lmfit-multi.c',
	'refl-multifit.c', 'disp-fit-engine.c',
	'vector_print.c', 'fit_result.c', 'writer.c', 'lexer.c'
]

libell = static_library('ell', libell_src,
    c_args: regpro_cargs + regpro_core_args,
    dependencies: regpro_core_deps
)
