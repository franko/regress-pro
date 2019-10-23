libcanvas_dep = dependency('libcanvas', required: false)

if libcanvas_dep.found()
    executable('kk-ho-compute', 'kk-ho-compute.cpp',
        dependencies: [gsl_dep, libcanvas_dep, thread_dep])
endif
