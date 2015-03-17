
BASEDIR = ..

include $(BASEDIR)/makeconfig
include $(BASEDIR)/makesystem

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)

ifeq ($(HOST_SYS),Windows)
  INCLUDES += -Ic:/fra/apps/include
  LIBS += -Lc:/fra/apps/lib

  FOX_HOME = c:/fra/src/fox-1.6.43
  FOX_LIBS = -L$(FOX_HOME)/src -lfox
  FOX_INCLUDES = -I$(FOX_HOME)/include
else
  FOX_LIBS = $(shell fox-config --libs)
  FOX_INCLUDES = $(shell fox-config --cflags)
endif

GSL_LIBS = -lgsl -lgslcblas -lm
GSL_INCLUDES = 

REG_LIBS = -lmhash -lgmp

LIBS += $(FOX_LIBS) $(GSL_LIBS) -lagg -lsupc++

ifeq ($(HOST_SYS),Windows)
  LIBS += -lgdi32 -lwsock32 -mwindows
endif

INCLUDES += $(FOX_INCLUDES) $(GSL_INCLUDES) -I..

CXXCOMPILE = $(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDES)

SRC_FILES = ProgressInfo.cpp spectrum_plot.cpp regress_pro_window.cpp \
	BatchDialog.cpp dispers_dialog.cpp dispers_win.cpp sampling_win.cpp \
	fx_numeric_field.cpp disp_chooser.cpp fit_window.cpp colors.cpp \
	plot_units.cpp plot_canvas.cpp utils.cpp filmstack_window.cpp \
	dispers_chooser.cpp dispers_ui_edit.cpp icons_all.cpp \
	fit_params_utils.cpp multi_sample_recipe.cpp recipe_window.cpp \
	fit_recipe.cpp dispers_edit_window.cpp filelist_table.cpp dataset_table.cpp \
	dataset_window.cpp main.cpp
PRG = regress.exe

REG_SRC_FILES := registration.c registered_app.cpp
REG_OBJ_FILEX := $(REG_SRC_FILES:%.cpp=%.o)
REG_OBJ_FILES := $(REG_OBJ_FILEX:%.c=%.o)

OBJ_FILES := $(SRC_FILES:%.cpp=%.o)
DEP_FILES := $(SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

ifeq ($(PRODUCT_REGISTRATION),yes)
  OBJ_FILES += $(REG_OBJ_FILES)  
  LIBS += $(REG_LIBS)
endif

.PHONY: clean all

all: $(PRG)

include $(BASEDIR)/makerules

$(PRG): $(OBJ_FILES) ../libefit.a
	$(CXX) -o $@ $(OBJ_FILES) ../libefit.a $(LIBS)

clean:
	rm -f $(OBJ_FILES) $(PRG)

-include $(DEP_FILES)
