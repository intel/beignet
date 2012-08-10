TOP=..

DIR_CXXFLAGS=-fexceptions

include ../Makefile.defs

SUBDIRS=.

C_SRC=utest_error.c

CPP_SRC=\
	app_mandelbrot.cpp \
	compiler_argument_structure.cpp \
	compiler_argument_structure_indirect.cpp \
	compiler_array0.cpp \
	compiler_array.cpp \
	compiler_byte_scatter.cpp \
	compiler_copy_buffer.cpp \
	compiler_copy_buffer_row.cpp \
	compiler_function_argument0.cpp \
	compiler_function_argument1.cpp \
	compiler_function_argument.cpp \
	compiler_if_else.cpp \
	compiler_local_slm.cpp \
	compiler_lower_return0.cpp \
	compiler_lower_return1.cpp \
	compiler_lower_return2.cpp \
	compiler_short_scatter.cpp \
	compiler_sub_bytes.cpp \
	compiler_sub_shorts.cpp \
	compiler_uint2_copy.cpp \
	compiler_uint3_copy.cpp \
	compiler_uint3_unaligned_copy.cpp \
	compiler_unstructured_branch0.cpp \
	compiler_unstructured_branch1.cpp \
	compiler_unstructured_branch2.cpp \
	compiler_unstructured_branch3.cpp \
	compiler_write_only_bytes.cpp \
	compiler_write_only.cpp \
	compiler_write_only_shorts.cpp \
	utest_assert.cpp \
	utest.cpp \
	utest_file_map.cpp \
	utest_helper.cpp

OBJ=$(C_SRC:.c=.o) $(CPP_SRC:.cpp=.o)

all: utest_run runtime_flat_address_space

utest_run: $(OBJ) utest_run.o $(TOP)/$(LIBBASE)
	$(CXX) -o $@ $(OBJ) utest_run.o $(TOP)/$(LIBBASE)

runtime_flat_address_space: $(OBJ) runtime_flat_address_space.o
	$(CXX) -o $@ $(OBJ) runtime_flat_address_space.o $(TOP)/$(LIBBASE)

clean:
	rm -f $(OBJ)
	rm -f utest_run utest_run.o
	rm -f runtime_flat_address_space runtime_flat_address_space.o