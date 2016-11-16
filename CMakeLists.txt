cmake_minimum_required(VERSION 2.8)

project(regress-pro)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package (GSL)
if (GSL_FOUND)
  include_directories(${GSL_INCLUDE_DIRS})
endif (GSL_FOUND)

set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -g -Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(SOURCE_REGPRO_CORE src/common.c
                       src/data-table.c
                       src/data-view.c
                       src/rc_matrix.c
                       src/disp-table.c
                       src/disp-sample-table.c
                       src/disp-lookup.c
                       src/str.c
                       src/dispers-library.c
                       src/str-util.c
                       src/batch.c
                       src/error-messages.c
                       src/cmpl.c
                       src/minsampling.c
                       src/dispers.c
                       src/disp-fb.c
                       src/disp-tauc-lorentz.c
                       src/disp-ho.c
                       src/disp-bruggeman.c
                       src/disp-cauchy.c
                       src/disp-sellmeier.c
                       src/dispers-classes.c
                       src/stack.c
                       src/lmfit.c
                       src/fit-timestamp.c
                       src/lmfit-simple.c
                       src/fit-params.c
                       src/fit-engine.c
                       src/refl-kernel.c
                       src/refl-fit.c
                       src/elliss-fit.c
                       src/number-parse.c
                       src/refl-utils.c
                       src/acquisition.c
                       src/spectra.c
                       src/elliss.c
                       src/test-deriv.c
                       src/elliss-multifit.c
                       src/multi-fit-engine.c
                       src/grid-search.c
                       src/lmfit-multi.c
                       src/refl-multifit.c
                       src/disp-fit-engine.c
                       src/vector_print.c
                       src/fit_result.c
                       src/writer.c
                       src/lexer.c
                       src/lmfit_result.c
)

add_library(efit STATIC ${SOURCE_REGPRO_CORE})
