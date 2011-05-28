#
# $Id: Makefile,v 1.20 2006/12/29 17:47:02 francesco Exp $
#

include makeconfig

# standard setting
ifeq ($(BUILD_FLAG), std)
  CC = gcc
  CFLAGS = -O2 -fomit-frame-pointer -finline-functions
endif
ifeq ($(BUILD_FLAG), pentium)
# i586 / Pentium optimized settings
  CC = gcc -march=i586
#  CFLAGS = -O2 -fomit-frame-pointer -malign-double
  CFLAGS = -O2 -fomit-frame-pointer
endif
ifeq ($(BUILD_FLAG), debug)
# DEBUG setting
  CC = gcc
#  CFLAGS = -g -Wall -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wnested-externs -Wno-sign-compare -Wno-unused-parameter -fno-common 
  CFLAGS = -g
  DEFS = -DDEBUG_MEM -DDEBUG_REGRESS
endif
ifeq ($(BUILD_FLAG), valgrind)
  CC = gcc -march=i586
  CFLAGS = -g -O2
endif

ifeq ($(WIN_FLAG), yes)
# Option for Windows Platform
  DEFS += -DWIN32
endif

# DEFS += -DYYDEBUG=1

GSL_LIBS = -lgsl -lgslcblas -lm
GSL_INCLUDES = 

LDFLAGS = $(GSL_LIBS)

LDFLAGS = $(GSL_LIBS)
INCLUDES = $(GSL_INCLUDES)

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)

ELL_SRC_FILES = common.c data-table.c data-view.c disp-table.c disp-sample-table.c disp-lookup.c str.c dispers-library.c str-util.c batch.c error-messages.c cmpl.c minsampling.c dispers.c disp-ho.c disp-bruggeman.c disp-cauchy.c dispers-classes.c stack.c lmfit.c fit-params.c disp-util.c fit-engine.c refl-kernel.c refl-fit.c elliss-fit.c refl-utils.c spectra.c elliss.c descr-util.c model-interp.c interp.c symtab.c descr.c fitlexer.c test-deriv.c elliss-multifit.c multi-fit-engine.c grid-search.c lmfit-multi.c sample-info.c multi-fit-interp.c refl-multifit.c
YACC_FILES = descr.y
EFIT_LIB = libefit.a
SUBDIRS = fox-gui

ELL_OBJ_FILES := $(ELL_SRC_FILES:%.c=%.o)
DEP_FILES := $(ELL_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

.PHONY: clean all $(SUBDIRS)

all: $(EFIT_LIB) $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

$(EFIT_LIB): $(ELL_OBJ_FILES)
	ar r $@ $(ELL_OBJ_FILES)

%.o: %.c
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

descr.c descr.h: descr.y
	bison -d -o descr.c descr.y

clean:
	rm -f $(EFIT_LIB) $(ELL_OBJ_FILES)

-include $(DEP_FILES)
