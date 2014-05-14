# Makefile for hello pass

# Path to top level of LLVM hierarchy
LEVEL = ../../..

# Test dir...
TEST_DIR = ../../tests

# debugger
DEBUGGER = gdb

# Name of the library to build
LIBRARYNAME = cs565_live_var_analyzer

# dir name based on llvm build type...
LLVM_BUILD_TYPE = Debug+Asserts

# path to llvm optimizer...
LLVM_OPT = $(LEVEL)/$(LLVM_BUILD_TYPE)/bin/opt

# Make the shared library become a loadable module so the tools can
# dlopen/dlsym on the resulting library.
LOADABLE_MODULE = 1

# Might need to change this to .so in LINUX
LIB_EXT = .so

LVA_ARGS = -load $(LEVEL)/$(LLVM_BUILD_TYPE)/lib/$(LIBRARYNAME)$(LIB_EXT) -analyze-live-var

# Include the makefile implementation stuff
include ../Makefile
include $(LEVEL)/Makefile.common

test:
	$(LLVM_OPT) $(LVA_ARGS) < $(TEST_DIR)/test3.bc > /dev/null

debug:
	gdb $(LLVM_OPT) -ex 'set breakpoint pending on' -ex 'set args $(LVA_ARGS) < $(TEST_DIR)/test3.bc > /dev/null' -x break.gdb
