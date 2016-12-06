
include makeconfig
include makesystem

GSL_LIBS = $(shell pkg-config --libs gsl)
GSL_INCLUDES = $(shell pkg-config --cflags gsl)

INCLUDES += $(GSL_INCLUDES)

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)

ELL_SRC_FILES = common.c data-table.c data-view.c rc_matrix.c disp-table.c \
	disp-sample-table.c disp-lookup.c str.c dispers-library.c str-util.c \
	batch.c error-messages.c cmpl.c minsampling.c dispers.c disp-fb.c disp-tauc-lorentz.c disp-ho.c \
	disp-bruggeman.c disp-cauchy.c disp-sellmeier.c dispers-classes.c stack.c lmfit.c \
	fit-timestamp.c lmfit-simple.c fit-params.c fit-engine.c refl-kernel.c \
	refl-fit.c elliss-fit.c number-parse.c refl-utils.c \
	acquisition.c \
	spectra.c elliss.c \
	elliss-multifit.c multi-fit-engine.c grid-search.c lmfit-multi.c \
	refl-multifit.c disp-fit-engine.c \
	vector_print.c fit_result.c writer.c lexer.c lmfit_result.c
EFIT_LIB = libefit.a

ELL_OBJ_FILES := $(ELL_SRC_FILES:%.c=%.o)
DEP_FILES := $(ELL_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

.PHONY: clean all

all: $(EFIT_LIB)

include makerules

$(SUBDIRS):
	$(MAKE) -C $@

$(EFIT_LIB): $(ELL_OBJ_FILES)
	ar r $@ $(ELL_OBJ_FILES)

clean:
	$(HOST_RM) $(ELL_OBJ_FILES) $(EFIT_LIB)

dispers_library_preload.h: dispers_library_preload.txt
	( echo "static const char *dispersions_data = \\"; cat $< | sed 's/"/\\"/g; s/\(.*\)/"\1\\n"/g'; echo ";" ) > $@

preset_library_data.h: preset_library_data.txt
	( echo "static const char *preset_library_data = \\"; cat $< | sed 's/"/\\"/g; s/\(.*\)/"\1\\n"/g'; echo ";" ) > $@

-include $(DEP_FILES)
