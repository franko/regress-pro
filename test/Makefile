#
# $Id: Makefile,v 1.1 2006/12/26 18:16:08 francesco Exp $
#

include ../makeconfig

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
  CFLAGS = -g -Wall -W -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wnested-externs -Wno-sign-compare -Wno-unused-parameter -fno-common 
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

GSL_LIBS = -lgsl -lgslcblas -lm
GSL_INCLUDES =

LDFLAGS = $(GSL_LIBS)

LDFLAGS = $(GSL_LIBS)
INCLUDES = $(GSL_INCLUDES) -I..

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)

TEST_SRC_FILES = gui-test.c
EFIT_LIB = ../libefit.a

TEST_OBJ_FILES := $(TEST_SRC_FILES:%.c=%.o)
DEP_FILES := $(TEST_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

.PHONY: clean all

all: gui-test.exe

gui-test.exe: $(TEST_OBJ_FILES) $(EFIT_LIB)
	$(CC) -o $@ $(TEST_OBJ_FILES) $(EFIT_LIB) $(LDFLAGS)

%.o: %.c
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

clean:
	rm -f $(TEST_OBJ_FILES) gui-test.exe

-include $(DEP_FILES)
