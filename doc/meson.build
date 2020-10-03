libelem_dep = dependency('libelem', required: false)

if libelem_dep.found()
    executable('kk-ho-compute', 'kk-ho-compute.cpp',
        dependencies: [gsl_dep, libelem_dep, thread_dep])
endif
