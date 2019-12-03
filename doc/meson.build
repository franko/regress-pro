libcanvas_dep = dependency('libcanvas', required: false)

if libcanvas_dep.found()
    executable('kk-ho-compute', 'kk-ho-compute.cpp',
        dependencies: [gsl_dep, libcanvas_dep, thread_dep])
    executable('kk-ho-2nd-order-compute', 'kk-ho-2nd-order-compute.cpp',
        dependencies: [gsl_dep, libcanvas_dep, thread_dep])
    executable('kk-kramers-test-compute', 'kk-kramers-test-compute.cpp',
        dependencies: [gsl_dep, libcanvas_dep, thread_dep])
endif
