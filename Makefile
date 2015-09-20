
include makeconfig
include makesystem

PACKAGE_NAME = regress-pro
VERSION = 2.0.1
DEBIAN_BUILD_DIR = debian_build
DEB_ARCH := $(shell dpkg-architecture -qDEB_HOST_ARCH)
DEBIAN_PACKAGE := $(PACKAGE_NAME)_$VERSION-1_$(DEB_ARCH).deb

GSL_LIBS = -lgsl -lgslcblas -lm
GSL_INCLUDES =

INCLUDES = $(GSL_INCLUDES)

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)

ELL_SRC_FILES = common.c data-table.c data-view.c rc_matrix.c disp-table.c \
	disp-sample-table.c disp-lookup.c str.c dispers-library.c str-util.c \
	batch.c error-messages.c cmpl.c minsampling.c dispers.c disp-fb.c disp-tauc-lorentz.c disp-ho.c \
	disp-bruggeman.c disp-cauchy.c dispers-classes.c stack.c lmfit.c \
	lmfit-simple.c fit-params.c fit-engine.c refl-kernel.c \
	refl-fit.c elliss-fit.c number-parse.c refl-utils.c spectra.c elliss.c test-deriv.c \
	elliss-multifit.c multi-fit-engine.c grid-search.c lmfit-multi.c \
	refl-multifit.c disp-fit-engine.c \
	vector_print.c fit_result.c writer.c lexer.c
EFIT_LIB = libefit.a
SUBDIRS = fox-gui

ELL_OBJ_FILES := $(ELL_SRC_FILES:%.c=%.o)
DEP_FILES := $(ELL_SRC_FILES:%.c=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

.PHONY: clean all $(SUBDIRS)

all: $(EFIT_LIB) $(SUBDIRS)

include makerules

$(SUBDIRS):
	$(MAKE) -C $@

$(EFIT_LIB): $(ELL_OBJ_FILES)
	ar r $@ $(ELL_OBJ_FILES)

clean:
	$(MAKE) -C fox-gui clean
	$(HOST_RM) $(ELL_OBJ_FILES) $(EFIT_LIB)
	$(HOST_RM) -r $(DEBIAN_BUILD_DIR)
	$(HOST_RM) $(DEBIAN_PACKAGE)

debian: $(DEBIAN_PACKAGE)

$(DEBIAN_PACKAGE): $(EFIT_LIB) fox-gui debian/control
	$(HOST_RM) -r $(DEBIAN_BUILD_DIR)
	mkdir -p $(DEBIAN_BUILD_DIR)/usr/bin $(DEBIAN_BUILD_DIR)/usr/share/applications $(DEBIAN_BUILD_DIR)/usr/share/icons/hicolor/128x128/apps $(DEBIAN_BUILD_DIR)/usr/share/regress-pro
	strip fox-gui/regress$(EXE)
	cp fox-gui/regress$(EXE) $(DEBIAN_BUILD_DIR)/usr/bin
	cp -R examples $(DEBIAN_BUILD_DIR)/usr/share/regress-pro
	cp regress-pro-128x128.png $(DEBIAN_BUILD_DIR)/usr/share/icons/hicolor/128x128/apps/regress-pro.png
	fakeroot bash debian/build.sh $(PACKAGE_NAME) $(VERSION)

dispers_library_preload.h: dispers_library_preload.txt
	( echo "static const char *dispersions_data = \\"; cat $< | sed 's/"/\\"/g; s/\(.*\)/"\1\\n"/g'; echo ";" ) > $@

preset_library_data.h: preset_library_data.txt
	( echo "static const char *preset_library_data = \\"; cat $< | sed 's/"/\\"/g; s/\(.*\)/"\1\\n"/g'; echo ";" ) > $@

-include $(DEP_FILES)
