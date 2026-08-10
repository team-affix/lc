# ==============================================================================
# Toolchain
# ==============================================================================

MAKEFLAGS += -j$(shell nproc)

CXX      = g++
DEPFLAGS = -MMD -MP
CXXFLAGS = -std=c++20 -Icore/hpp $(DEPFLAGS)
AR       = ar
ARFLAGS  = rcs

DEBUG_CXXFLAGS      = -DDEBUG -g
DEBUG_FAST_CXXFLAGS = -DDEBUG -g -O3
PROFILE_CXXFLAGS    = -DDEBUG -g -O3 -pg
RELEASE_CXXFLAGS    = -O3

# ==============================================================================
# Paths
# ==============================================================================

GTEST_INC      = googletest/googletest/include
GMOCK_INC      = googletest/googlemock/include
GTEST_ALL_CC   = googletest/googletest/src/gtest-all.cc
GMOCK_ALL_CC   = googletest/googlemock/src/gmock-all.cc
GTEST_CPPFLAGS = \
    -I$(GTEST_INC) -I$(GMOCK_INC) \
    -Igoogletest/googletest -Igoogletest/googlemock

# ==============================================================================
# Output names  (all under build/)
# ==============================================================================

CORE_LIB            = build/liblc_core.a
CORE_DEBUG_LIB      = build/liblc_core_debug.a
CORE_DEBUG_FAST_LIB = build/liblc_core_debug_fast.a
CORE_PROFILE_LIB    = build/liblc_core_profile.a

CORE_DEBUG_BIN      = build/core_debug
CORE_DEBUG_FAST_BIN = build/core_debug_fast
CORE_PROFILE_BIN    = build/core_profile

# ==============================================================================
# Object lists
# ==============================================================================

CORE_SRC = $(shell find core/cpp -name '*.cpp' | sort)

CORE_OBJ            = $(patsubst core/cpp/%.cpp, build/obj/core/%.o,            $(CORE_SRC))
CORE_DEBUG_OBJ      = $(patsubst core/cpp/%.cpp, build/obj/core_debug/%.o,      $(CORE_SRC))
CORE_DEBUG_FAST_OBJ = $(patsubst core/cpp/%.cpp, build/obj/core_debug_fast/%.o, $(CORE_SRC))
CORE_PROFILE_OBJ    = $(patsubst core/cpp/%.cpp, build/obj/core_profile/%.o,    $(CORE_SRC))

CORE_TEST_SRC = $(shell find core/test -name '*.cpp' | sort)

CORE_DEBUG_TEST_OBJ = \
    $(patsubst core/test/%.cpp, build/obj/core_debug_test/%.o, $(CORE_TEST_SRC))
CORE_DEBUG_FAST_TEST_OBJ = \
    $(patsubst core/test/%.cpp, build/obj/core_debug_fast_test/%.o, $(CORE_TEST_SRC))
CORE_PROFILE_TEST_OBJ = \
    $(patsubst core/test/%.cpp, build/obj/core_profile_test/%.o, $(CORE_TEST_SRC))

CORE_DEBUG_GTEST_OBJ = \
    build/obj/core_debug_test/gtest-all.o \
    build/obj/core_debug_test/gmock-all.o
CORE_DEBUG_FAST_GTEST_OBJ = \
    build/obj/core_debug_fast_test/gtest-all.o \
    build/obj/core_debug_fast_test/gmock-all.o
CORE_PROFILE_GTEST_OBJ = \
    build/obj/core_profile_test/gtest-all.o \
    build/obj/core_profile_test/gmock-all.o

CORE_DEBUG_BIN_OBJ      = $(CORE_DEBUG_TEST_OBJ) $(CORE_DEBUG_GTEST_OBJ)
CORE_DEBUG_FAST_BIN_OBJ = $(CORE_DEBUG_FAST_TEST_OBJ) $(CORE_DEBUG_FAST_GTEST_OBJ)
CORE_PROFILE_BIN_OBJ    = $(CORE_PROFILE_TEST_OBJ) $(CORE_PROFILE_GTEST_OBJ)

CORE_DEBUG_TEST_CXXFLAGS      = $(DEBUG_CXXFLAGS) $(GTEST_CPPFLAGS) -Icore/test
CORE_DEBUG_FAST_TEST_CXXFLAGS = $(DEBUG_FAST_CXXFLAGS) $(GTEST_CPPFLAGS) -Icore/test
CORE_PROFILE_TEST_CXXFLAGS    = $(PROFILE_CXXFLAGS) $(GTEST_CPPFLAGS) -Icore/test

CORE_DEP                 = $(CORE_OBJ:.o=.d)
CORE_DEBUG_DEP           = $(CORE_DEBUG_OBJ:.o=.d)
CORE_DEBUG_FAST_DEP      = $(CORE_DEBUG_FAST_OBJ:.o=.d)
CORE_PROFILE_DEP         = $(CORE_PROFILE_OBJ:.o=.d)
CORE_DEBUG_TEST_DEP      = $(CORE_DEBUG_TEST_OBJ:.o=.d)
CORE_DEBUG_FAST_TEST_DEP = $(CORE_DEBUG_FAST_TEST_OBJ:.o=.d)
CORE_PROFILE_TEST_DEP    = $(CORE_PROFILE_TEST_OBJ:.o=.d)
CORE_DEBUG_GTEST_DEP      = $(CORE_DEBUG_GTEST_OBJ:.o=.d)
CORE_DEBUG_FAST_GTEST_DEP = $(CORE_DEBUG_FAST_GTEST_OBJ:.o=.d)
CORE_PROFILE_GTEST_DEP    = $(CORE_PROFILE_GTEST_OBJ:.o=.d)

# ==============================================================================
# User-facing targets
# ==============================================================================

.PHONY: all core core_debug core_debug_fast core_profile clean

all: core core_debug core_debug_fast

core: $(CORE_LIB)

core_debug: $(CORE_DEBUG_BIN)

core_debug_fast: $(CORE_DEBUG_FAST_BIN)

# Profile entrypoint: -O3 -g -pg for gprof hot-path analysis (no perf/gdb attach needed).
core_profile: $(CORE_PROFILE_BIN)

clean:
	rm -rf build

# ==============================================================================
# Library archive rules
# ==============================================================================

$(CORE_LIB): $(CORE_OBJ) | build
	$(AR) $(ARFLAGS) $@ $^

$(CORE_DEBUG_LIB): $(CORE_DEBUG_OBJ) | build
	$(AR) $(ARFLAGS) $@ $^

$(CORE_DEBUG_FAST_LIB): $(CORE_DEBUG_FAST_OBJ) | build
	$(AR) $(ARFLAGS) $@ $^

$(CORE_PROFILE_LIB): $(CORE_PROFILE_OBJ) | build
	$(AR) $(ARFLAGS) $@ $^

# ==============================================================================
# Binary link rules
# ==============================================================================

$(CORE_DEBUG_BIN): $(CORE_DEBUG_LIB) $(CORE_DEBUG_BIN_OBJ) | build
	$(CXX) $(CXXFLAGS) -o $@ \
	    $(CORE_DEBUG_BIN_OBJ) \
	    -Lbuild -llc_core_debug -lpthread

$(CORE_DEBUG_FAST_BIN): $(CORE_DEBUG_FAST_LIB) $(CORE_DEBUG_FAST_BIN_OBJ) | build
	$(CXX) $(CXXFLAGS) -o $@ \
	    $(CORE_DEBUG_FAST_BIN_OBJ) \
	    -Lbuild -llc_core_debug_fast -lpthread

$(CORE_PROFILE_BIN): $(CORE_PROFILE_LIB) $(CORE_PROFILE_BIN_OBJ) | build
	$(CXX) $(CXXFLAGS) $(PROFILE_CXXFLAGS) -o $@ \
	    $(CORE_PROFILE_BIN_OBJ) \
	    -Lbuild -llc_core_profile -lpthread

# ==============================================================================
# Compilation pattern rules
# ==============================================================================

build/obj/core/%.o: core/cpp/%.cpp | build/obj/core
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(RELEASE_CXXFLAGS) -c $< -o $@

build/obj/core_debug/%.o: core/cpp/%.cpp | build/obj/core_debug
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEBUG_CXXFLAGS) -c $< -o $@

build/obj/core_debug_fast/%.o: core/cpp/%.cpp | build/obj/core_debug_fast
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEBUG_FAST_CXXFLAGS) -c $< -o $@

build/obj/core_profile/%.o: core/cpp/%.cpp | build/obj/core_profile
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(PROFILE_CXXFLAGS) -c $< -o $@

build/obj/core_debug_test/%.o: core/test/%.cpp | build/obj/core_debug_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_DEBUG_TEST_CXXFLAGS) -c $< -o $@

build/obj/core_debug_test/gtest-all.o: $(GTEST_ALL_CC) | build/obj/core_debug_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_DEBUG_TEST_CXXFLAGS) -c $< -o $@

build/obj/core_debug_test/gmock-all.o: $(GMOCK_ALL_CC) | build/obj/core_debug_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_DEBUG_TEST_CXXFLAGS) -c $< -o $@

build/obj/core_debug_fast_test/%.o: core/test/%.cpp | build/obj/core_debug_fast_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_DEBUG_FAST_TEST_CXXFLAGS) -c $< -o $@

build/obj/core_debug_fast_test/gtest-all.o: $(GTEST_ALL_CC) | build/obj/core_debug_fast_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_DEBUG_FAST_TEST_CXXFLAGS) -c $< -o $@

build/obj/core_debug_fast_test/gmock-all.o: $(GMOCK_ALL_CC) | build/obj/core_debug_fast_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_DEBUG_FAST_TEST_CXXFLAGS) -c $< -o $@

build/obj/core_profile_test/%.o: core/test/%.cpp | build/obj/core_profile_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_PROFILE_TEST_CXXFLAGS) -c $< -o $@

build/obj/core_profile_test/gtest-all.o: $(GTEST_ALL_CC) | build/obj/core_profile_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_PROFILE_TEST_CXXFLAGS) -c $< -o $@

build/obj/core_profile_test/gmock-all.o: $(GMOCK_ALL_CC) | build/obj/core_profile_test
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_PROFILE_TEST_CXXFLAGS) -c $< -o $@

# ==============================================================================
# Directories
# ==============================================================================

build build/obj/core build/obj/core_debug build/obj/core_debug_fast \
build/obj/core_profile build/obj/core_debug_test build/obj/core_debug_fast_test \
build/obj/core_profile_test:
	mkdir -p $@

# ==============================================================================
# Header dependencies
# ==============================================================================

-include $(CORE_DEP)
-include $(CORE_DEBUG_DEP)
-include $(CORE_DEBUG_FAST_DEP)
-include $(CORE_PROFILE_DEP)
-include $(CORE_DEBUG_TEST_DEP)
-include $(CORE_DEBUG_FAST_TEST_DEP)
-include $(CORE_PROFILE_TEST_DEP)
-include $(CORE_DEBUG_GTEST_DEP)
-include $(CORE_DEBUG_FAST_GTEST_DEP)
-include $(CORE_PROFILE_GTEST_DEP)
