# Makefile for hello pass

# Path to top level of LLVM hierarchy
LEVEL = ../../..

# Test dir...
TEST_DIR = ../../tests

# Name of the library to build
LIBRARYNAME = cs565_pretty_code

# dir name based on llvm build type...
LLVM_BUILD_TYPE = Debug+Asserts

# Make the shared library become a loadable module so the tools can
# dlopen/dlsym on the resulting library.
LOADABLE_MODULE = 1

# Might need to change this to .so in LINUX
LIB_EXT = .so

# Include the makefile implementation stuff
include ../Makefile
include $(LEVEL)/Makefile.common

test:
	$(TEST_PRINT) $(TEST_DIR)/test1.bc > /dev/null
