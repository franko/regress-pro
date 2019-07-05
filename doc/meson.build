libcanvas_dep = dependency('libcanvas', required: false)
libcanvaswin_dep = dependency('libcanvaswin', required: false)

if libcanvas_dep.found() and libcanvaswin_dep.found()
    executable('kk-ho-compute', 'kk-ho-compute.cpp',
        dependencies: [gsl_dep, libcanvas_dep, libcanvaswin_dep, thread_dep])
endif
