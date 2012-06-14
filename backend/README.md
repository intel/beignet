OpenCL Compiler
===============

This code base contains the compiler part of the complete OpenCL stack. The
run-time is not in this code base and is developed inside "OpenCL runtime"
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

`> make`

The project only depends on LLVM (Low-Level Virtual Machine project). Right
now, the code has only been compiled with LLVM 3.0. It will not compile with
any thing older. A small amount of work should be required to make it work
with LLVM 3.1 but the port is not done. LLVM 3.0 can be downloaded at:

[http://llvm.org/releases/](http://llvm.org/releases/)

Be careful and download LLVM *3.0*

How to run
----------

There is nothing to do to run it. However, unit tests can be compiled (by
setting `GBE_COMPILE_UTESTS` to true). Once compiled, you can simply run

`> ./tester # to run the unit tests`

Limitations
-----------

Today, the compiler is far from complete. See doc/TODO.md for a (incomplete)
lists of things to do.

Implementation details
----------------------

Several key decisions may use the hardware in an usual way. See the following
documents for the technical details about the compiler implementation:

- [Flat address space](doc/flat\_address\_space.html)
- [Unstructured branches](doc/unstructured\_branches.html)
- [Scalar intermediate representation](doc/gen\_ir.html)
- [Clean backend implementation](doc/backend.html)

Ben Segovia (<benjamin.segovia@intel.com>)

