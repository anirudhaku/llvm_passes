# Makefile for hello pass

# Path to top level of LLVM hierarchy
LEVEL = ../../..

# Test dir...
TEST_DIR = ../../tests

# Name of the library to build
LIBRARYNAME = cs565_naive_trimmer

# dir name based on llvm build type...
LLVM_BUILD_TYPE = Debug+Asserts

# Make the shared library become a loadable module so the tools can
# dlopen/dlsym on the resulting library.
LOADABLE_MODULE = 1

# Might need to change this to .so in LINUX
LIB_EXT = .so

TEST_NAIVE = $(LEVEL)/$(LLVM_BUILD_TYPE)/bin/opt -load $(LEVEL)/$(LLVM_BUILD_TYPE)/lib/$(LIBRARYNAME)$(LIB_EXT) -naive-trim <

# Include the makefile implementation stuff
include ../Makefile
include $(LEVEL)/Makefile.common

test:
	$(TEST_PRINT) $(TEST_DIR)/test2.bc > /dev/null
	$(TEST_NAIVE) $(TEST_DIR)/test2.bc > $(TEST_DIR)/test2.opt.bc
	$(TEST_PRINT) $(TEST_DIR)/test2.opt.bc > /dev/null
