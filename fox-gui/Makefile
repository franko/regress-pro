
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

  AGG_INCLUDES = -I/usr/include/agg2
endif

GSL_LIBS = -lgsl -lgslcblas -lm
GSL_INCLUDES = 

REG_LIBS = -lmhash -lgmp

LIBS = $(FOX_LIBS) $(GSL_LIBS) $(REG_LIBS) -lsupc++

ifeq ($(HOST_SYS),Windows)
  LIBS += -lgdi32 -lwsock32
endif

INCLUDES = $(FOX_INCLUDES) $(GSL_INCLUDES) $(AGG_INCLUDES) -I..

CXXCOMPILE = $(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDES)

SRC_FILES = units.cpp ProgressInfo.cpp EllissGui.cpp BatchDialog.cpp DispersDialog.cpp DispersWin.cpp SpectrRangeWin.cpp spectra-path.cpp InteractiveFit.cpp registered_app.cpp main.cpp
PRG = regress.exe

REG_SRC_FILES = registration.c
REG_OBJ_FILES := $(REG_SRC_FILES:%.c=%.o)

OBJ_FILES := $(SRC_FILES:%.cpp=%.o)
DEP_FILES := $(SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

.PHONY: clean all

all: $(PRG)

include $(BASEDIR)/makerules

$(PRG): $(OBJ_FILES) $(REG_OBJ_FILES) ../libefit.a
	$(CXX) -o $@ $(OBJ_FILES) $(REG_OBJ_FILES) ../libefit.a $(LIBS)

clean:
	rm -f $(OBJ_FILES) $(REG_OBJ_FILES) $(PRG)

-include $(DEP_FILES)
