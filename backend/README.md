OpenCL Compiler
===============

This code base contains the compiler part of the OpenCL stack. The
run-time is not in this code base and is developed inside "OpenCL run-time"
project. The compiler is responsible to take a OpenCL language string and to
compile it into a binary that can be executed on Intel integrated GPUs.

How to build
------------

The project uses CMake with three profiles:

  1. Debug (-g)
  2. RelWithDebInfo (-g with optimizations)
  3. Release (only optimizations)

Basically, from the root directory of the project

`> mkdir build`

`> cd build`

`> ccmake ../ # to configure`

Choose whatever you want for the build

Then press 'c' to configure and 'g' to generate the code

`> make # to build it`

`> make install # to copy it in the proper place`

The project only depends on LLVM (Low-Level Virtual Machine project). Right
now, the code has only been compiled with LLVM 3.0. It will not compile with
any thing older. A small amount of work should be required to make it work
with LLVM 3.1 but the port is not done. LLVM 3.0 can be downloaded at:

[http://llvm.org/releases/](http://llvm.org/releases/)

Be careful and download LLVM *3.0*

Also note that the code was compiled on GCC 4.6 and GCC 4.7. Since the code uses
really recent C++11 features, you may expect problems with older compilers. Last
time I tried, the code breaks ICC 12 and Clang with internal compiler errors
while compiling anonymous nested lambda functions.

How to run
----------

There is nothing to do to run it. However, unit tests can be compiled (by
setting `GBE_COMPILE_UTESTS` to true). Once compiled, you can simply run

`> ./tester # to run the unit tests`

Supported Hardware
------------------

Only IVB is supported right now. Actually, the code was only run on IVB GT2. You
may expect some issues with IVB GT1. Also, the code should work as it is on HSW
but it was not tested so you should expect number of issues with the HSW
platform.

Limitations
-----------

Today, the compiler is far from complete. See [here](doc/TODO.html) for a
(incomplete) lists of things to do.

Interface with the run-time
---------------------------

Even if the compiler makes a very liberal use of C++ (templates, variadic
templates, macros), we really tried hard to make a very simple interface with
the run-time. The interface is therefore a pure C99 interface and it is defined
in `src/backend/program.h`.

The goal is to hide the complexity of the inner data structures and to enable
simple run-time implementation using straightforward C99.

Note that the data structures are fully opaque: this allows us to use both the
C++ simulator or the real Gen program in a relatively non-intrusive way.

Various environment variables
-----------------------------

Environment variables are used all over the code. Most important ones are:

- `OCL_SIMD_WIDTH` `(8 or 16)`. Change the number of lanes per hardware thread

- `OCL_OUTPUT_GEN_IR` `(0 or 1)`. Output Gen IR (scalar intermediate
  representation) code

- `OCL_OUTPUT_LLVM` `(0 or 1)`. Output LLVM code after the lowering passes

- `OCL_OUTPUT_LLVM_BEFORE_EXTRA_PASS` `(0 or 1)`. Output LLVM code before the
  lowering passes

- `OCL_OUTPUT_ASM` `(0 or 1)`. Output Gen ISA

Implementation details
----------------------

Several key decisions may use the hardware in an usual way. See the following
documents for the technical details about the compiler implementation:

- [Flat address space](doc/flat\_address\_space.html)
- [Unstructured branches](doc/unstructured\_branches.html)
- [Scalar intermediate representation](doc/gen\_ir.html)
- [Clean backend implementation](doc/compiler_backend.html)
- [C++ OpenCL simulator](doc/c++_simulator.html)

Ben Segovia (<benjamin.segovia@intel.com>)

