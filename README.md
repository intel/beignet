Beignet
=======

Beignet is an open source implementaion of the OpenCL specification - a generic
compute oriented API. This code base contains the code to run OpenCL programs on
Intel GPUs which bsically defines and implements the OpenCL host functions
required to initialize the device, create the command queues, the kernels and
the programs and run them on the GPU. The code base also contains the compiler
part of the stack which is included in `backend/`. For more specific information
about the compiler, please refer to `backend/README.md`

How to build
------------

There are two ways to build Beignet.

The first one uses a simple Makefile. Just type `make` and the project will
build if everything is properly installed.

The project also uses CMake with three profiles:

1. Debug (-g)
2. RelWithDebInfo (-g with optimizations)
3. Release (only optimizations)

Basically, from the root directory of the project

`> mkdir build`

`> cd build`

`> cmake ../ # to configure`

Choose whatever you want for the build.

Then press 'c' to configure and 'g' to generate the code.

`> make`

The project depends on several external libraries:

- Several X components (XLib, Xfixes, Xext)
- libdrm libraries (libdrm and libdrm\_intel)
- Various LLVM components
- The compiler backend itself (libgbe)
- Mesa git master version built with gbm enabled to support extension cl\_khr\_gl\_sharing.

CMake will check the dependencies and will complain if it does not find them.

The cmake will also build the backend project. Please refer to:
[OpenCL Gen Backend](backend/README.html) to get more dependencies.

Once built, the run-time produces a shared object libcl.so which basically
directly implements the OpenCL API. A set of tests are also produced. They may
be found in `utests/`.

Note that the compiler depends on LLVM (Low-Level Virtual Machine project).
Right now, the code has been compiled with LLVM 3.1/3.2. It will not compile
with any thing older. 

[http://llvm.org/releases/](http://llvm.org/releases/)

LLVM 3.1 and 3.2 are supported.

Also note that the code was compiled on GCC 4.6 and GCC 4.7. Since the code uses
really recent C++11 features, you may expect problems with older compilers. Last
time I tried, the code breaks ICC 12 and Clang with internal compiler errors
while compiling anonymous nested lambda functions.

How to run
----------

Apart from the OpenCL library itself that can be used by any OpenCL application,
this code also produces various tests to ensure the compiler and the run-time
consistency. This small test framework uses a simple c++ registration system to
register all the unit tests.

You need to set the variable `OCL_KERNEL_PATH` to locate the OCL kernels. They
are with the run-time in `./kernels`.

Then in `utests/`:

`> ./utest_run`

will run all the unit tests one after the others

`> ./utest_run some_unit_test0 some_unit_test1`

will only run `some_unit_test0` and `some_unit_test1` tests

Supported Hardware
------------------

As an important remark, the code was only tested on IVB GT2 with a rather
minimal Linux distribution (ArchLinux) and a very small desktop (dwm). If you
use something more sophisticated using compiz or similar stuffs, you may expect
serious problems and GPU hangs.

Only IVB is supported right now. Actually, the code was only run on IVB GT2. You
may expect some issues with IVB GT1.

TODO
----

The run-time is far from being complete. Most of the pieces have been put
together to test and develop the OpenCL compiler. A partial list of things to
do:

- Support dynamic sampler assignment in kernel. We now only support user to use
  clCreateSampler at host side and then pass the sampler to kernel.

- Complete cl\_khr\_gl\_sharing support. We lack of some APIs implementation such
  as clCreateFromGLBuffer,clCreateFromGLRenderbuffer,clGetGLObjectInfo... Currently,
  the working APIs are clCreateFromGLTexture,clCreateFromGLTexture2D.

- Support for events.

- Check that NDRangeKernels can be pushed into _different_ queues from several
  threads.

- Support for nonblocking mode Enqueue\*Buffer. Now we only use the map extension to
  implement those Enqueue\*Buffer functions. 

- No state tracking at all. One batch buffer is created at each "draw call"
  (i.e. for each NDRangeKernels). This is really inefficient since some
  expensive pipe controls are issued for each batch buffer

- Valgrind reports some leaks in libdrm. It sounds like a false positive but it
  has to be checked. Idem for LLVM. There is one leak here to check.

- Support image/samplers in C++ simulator.

More generally, everything in the run-time that triggers the "FATAL" macro means
that something that must be supported is not implemented properly (either it
does not comply with the standard or it is just missing)

Project repository

Right now, we host our project on fdo at: git://anongit.freedesktop.org/beignet.

The team
--------
This project was created by Ben Segovia when he was working for Intel. Now we
have a team in China OTC graphics department continue to work on this project.
We haven't set up a public mail list for this project, but we will do so in
the near furture. Before that, the contact is as below:
Zou Nanhai (<nanhai.zou@intel.com>).
