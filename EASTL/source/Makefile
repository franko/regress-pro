CXX = g++ -std=c++11
CXXFLAGS = -O2 -ffast-math -fomit-frame-pointer

INCLUDES = -I../include -I../test/packages/EABase/include/Common

CXXCOMPILE = $(CXX) $(CXXFLAGS) $(INCLUDES)

SRC_FILES = allocator_eastl.cpp assert.cpp fixed_pool.cpp hashtable.cpp intrusive_list.cpp numeric_limits.cpp red_black_tree.cpp string.cpp thread_support.cpp
OBJ_FILES := $(SRC_FILES:%.cpp=%.o)
DEP_FILES := $(ELL_SRC_FILES:%.cpp=.deps/%.P)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

.PHONY: clean all

all: libeastl.a

clean:
	rm *.o

libeastl.a: $(OBJ_FILES)
	ar r $@ $(OBJ_FILES)

%.o: %.cpp
	@echo '$(CXXCOMPILE) -c $<'; \
	$(CXXCOMPILE) -Wp,-MMD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
          | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
            >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

-include $(DEP_FILES)
