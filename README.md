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

`> ccmake ../ # to configure`

Choose whatever you want for the build.

Then press 'c' to configure and 'g' to generate the code.

`> make`

The project depends on several external libraries:

- Several X components (XLib, Xfixes, Xext)
- libdrm libraries (libdrm and libdrm\_intel)
- Various LLVM components

CMake will check the dependencies and will complain if it does not find them.

Once built, the run-time produces a shared object libcl.so which basically
directly implements the OpenCL API. A set of tests are also produced. They may
be found in `utests/`.

Note that the compiler depends on LLVM (Low-Level Virtual Machine project).
Right now, the code has only been compiled with LLVM 3.0. It will not compile
with any thing older. 

[http://llvm.org/releases/](http://llvm.org/releases/)

LLVM 3.0 and 3.1 are supported. LLVM 3.2 is partially supported right now. More
work is needed to make it fully work.

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

- Support for samplers / textures but it should be rather easy since the
  low-level parts of the code already supports it

- Support for events

- Check that NDRangeKernels can be pushed into _different_ queues from several
  threads 

- Support for Enqueue\*Buffer. I added a straightforward extension to map /
  unmap buffer. This extension `clIntelMapBuffer` directly maps `dri_bo_map`
  which is really convenient

- Full support for images. Today, the code just tiles everything *manually*
  which is really bad. I think the best solution to copy and create images is to
  use the GPU and typed writes (scatter to textures) or samplers. We would
  however need the vmap extension proposed by Chris Wilson to be able to map
  user pointers while doing to copies and the conversions.

- No state tracking at all. One batch buffer is created at each "draw call"
  (i.e. for each NDRangeKernels). This is really inefficient since some
  expensive pipe controls are issued for each batch buffer

- Valgrind reports some leaks in libdrm. It sounds like a false positive but it
  has to be checked. Idem for LLVM. There is one leak here to check

More generally, everything in the run-time that triggers the "FATAL" macro means
that something that must be supported is not implemented properly (either it
does not comply with the standard or it is just missing)

Ben Segovia (<benjamin.segovia@intel.com>)

