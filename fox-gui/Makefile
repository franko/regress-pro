
#
# $Id: Makefile,v 1.4 2006/07/13 14:32:21 francesco Exp $
#


include ../makeconfig

# standard setting
ifeq ($(BUILD_FLAG), std)
  CXX = g++
  CXXFLAGS = -O2 -finline-functions
endif
ifeq ($(BUILD_FLAG), pentium)
# i586 / Pentium optimized settings
  CXX = g++ -march=i586
  CXXFLAGS = -O2 -finline-functions
endif
ifeq ($(BUILD_FLAG), debug)
# DEBUG setting
  CXX = g++
  CXXFLAGS = -g -Wall -W -fno-inline 
  DEFS = -DDEBUG_MEM
endif

ifeq ($(WIN_FLAG), yes)
# Option for Windows Platform
  DEFS += -DWIN32
endif

# FOX_HOME = c:/fra/src/fox-1.6.43
# FOX_LIBS = -L$(FOX_HOME)/src -lfox
# FOX_INCLUDES = -I$(FOX_HOME)/include

FOX_LIBS = $(shell fox-config --libs)
FOX_INCLUDES = $(shell fox-config --cflags)

GSL_LIBS = -lgsl -lgslcblas -lm
GSL_INCLUDES = 

#  -lsupc++ -lgdi32 -mwindows 

LDFLAGS = $(FOX_LIBS) $(GSL_LIBS) 

ifeq ($(WIN_FLAG), yes)
  LDFLAGS += -lgdi32 -lwsock32
endif

INCLUDES = $(FOX_INCLUDES) $(GSL_INCLUDES) -I..

CXXCOMPILE = $(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDES)

SRC_FILES = units.cpp scale.cpp fxscale.cpp ProgressInfo.cpp EllissGui.cpp BatchDialog.cpp DispersDialog.cpp DispersWin.cpp SpectrRangeWin.cpp main.cpp
PRG = regress.exe

OBJ_FILES := $(SRC_FILES:%.cpp=%.o)
DEP_FILES := $(SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

.PHONY: clean all

all: $(PRG)

$(PRG): $(OBJ_FILES) ../libefit.a
	$(CXX) -o $@ $(OBJ_FILES) ../libefit.a $(LDFLAGS)

%.o: %.cpp
	@echo '$(CXXCOMPILE) -c $<'; \
	$(CXXCOMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

clean:
	rm -f $(OBJ_FILES) $(PRG)

-include $(DEP_FILES)
