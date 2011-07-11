
BASEDIR = ..

include $(BASEDIR)/makeconfig
include $(BASEDIR)/makesystem

COMPILE = $(CC) $(CFLAGS) $(DEFS) $(INCLUDES)

ifeq ($(HOST_SYS),Windows)
  DEFS += -DWIN32

  FOX_HOME = c:/fra/src/fox-1.6.43
  FOX_LIBS = -L$(FOX_HOME)/src -lfox
  FOX_INCLUDES = -I$(FOX_HOME)/include

  AGG_INCLUDES = -Ic:/fra/src/agg-2.5/include
else
  FOX_LIBS = $(shell fox-config --libs)
  FOX_INCLUDES = $(shell fox-config --cflags)
endif

GSL_LIBS = -lgsl -lgslcblas -lm
GSL_INCLUDES = 

REG_LIBS = -lmhash -lgmp

LIBS = $(FOX_LIBS) $(GSL_LIBS) -lsupc++

ifeq ($(HOST_SYS),Windows)
  LIBS += -lgdi32 -lwsock32
endif

INCLUDES = $(FOX_INCLUDES) $(GSL_INCLUDES) $(AGG_INCLUDES) -I..

CXXCOMPILE = $(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDES)

SRC_FILES = units.cpp ProgressInfo.cpp regress_pro_window.cpp BatchDialog.cpp DispersDialog.cpp DispersWin.cpp SpectrRangeWin.cpp spectra-path.cpp fx_numeric_field.cpp interactive_fit.cpp disp_chooser.cpp disp_fit_window.cpp main.cpp
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
