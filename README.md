OpenCL Runtime
==============

This code base contains the code to run OpenCL programs on Intel GPUs. This is
basically the run-time code i.e. it defines the OpenCL host functions required
to initialize the device, create the command queues, the kernels and the
programs and run them on the GPU. The run-time does *not* contain the compiler.
The OpenCL compiler has its own shared object and both the run-time and the
compiler are interfaced with a regular C layer.

How to build
------------

The project uses CMake with three profiles:

1. Debug (-g)
2. RelWithDebInfo (-g with optimizations)
3. Release (only optimizations)

Basically, from the root directory of the project

`> mkdir build`

`> ccmake ../ # to configure`

Choose whatever you want for the build.

Then press 'c' to configure and 'g' to generate the code.

`> make`

The project depends on several external libraries:

- Several X components (XLib, Xfixes, Xext)
- libdrm libraries (libdrm and libdrm\_intel)
- The compiler backend itself (libgbe)

CMake will check the dependencies and will complain if it does not find them.

Once built, the run-time produces a shared object libcl.so which basically
directly implements the OpenCL API. A set of tests are also produced. They may
be found in utests.

How to run
----------

Apart from the OpenCL library itself that can be used by any OpenCL application,
this code also produces various tests to ensure the compiler and the run-time
consistency. This small test framework uses a simple c++ registration system to
register all the unit tests.

You need to set the variable `OCL_KERNEL_PATH` to locate the OCL kernels. They
are with the run-time in `./kernels`.

Then in `utests/`:

`> ./run`

will run all the unit tests one after the others

`> ./run some_unit_test0 some_unit_test1`

will only run `some_unit_test0` and `some_unit_test1` tests

As an important remark, the code was only tested on IVB GT2 with a rather
minimal Linux distribution (ArchLinux) and a very small desktop (dwm). If you
use something more sophisticated using compiz or similar stuffs, you may expect
serious problems and GPU hangs.

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

Fulsim
------

The code base supports a seamless integration with Fulsim i.e. you do not need
to run anything else than your application to make Fulsim work with it. However,
some specific step have to be completed first to make it work.

- Compilation phase. You need to compile the project with fulsim enabled. You
should choose `EMULATE_IVB ON` in ccmake options. Actually, Haswell has not been
tested that much recently so there is a large probability it will not work
properly

- Fulsim executables and DLL. Copy and paste fulsim *Windows* executables and
DLLs into the directory where you run your code. The run-time will simply call
AubLoad.exe to run Fulsim. You can get fulsim from our subversion server. We
compile versions of it. They are all located in
[here](https://subversion.jf.intel.com/cag/gen/gpgpu/fulsim/)

- Run-time phase. You need to fake the machine you want to simulate. Small
scripts in the root directory of the project are responsible for doing that:

`> source setup_fulsim_ivb.sh 1`

will run fulsim in debug mode i.e. you will be able to step into the EU code

`> source setup_fulsim_ivb.sh 0`

will simply run fulsim

- Modified libdrm. Unfortunately, to support fulsim, this run-time uses a
modified libdrm library (in particular to support binary buffers and a seamless
integration with the run-time). See below.

C++ simulator
-------------

The compiler is able to produce c++ file that simulate the behavior of the
kernel. The idea is mostly to be able to gather statistics about how the kernel
can run (SIMD occupancy, bank conflicts in shared local memory or cache hit/miss
rates). Basically, the compiler generates a c++ file from the LLVM file (with
some extra steps detailed in the OpenCL compiler documentation). Then, GCC (or
ICC) is directly called to generate a shared object.

The run-time is actually able to run the simulation code directly. To enable it
(and to also enable the c++ path in the compile code), a small script in the
root directory has to be run:

`> source setup_perfim_ivb.sh`

Doing that, the complete C++ simulation path is enabled.

Modified libdrm
---------------

Right now, a modified libdrm is required to run fulsim. It completely disables
the HW path (nothing will run on the HW at all) and allows to selectively dump
any OpenCL buffer. Contact Ben Segovia to get the access to it.

Ben Segovia (<benjamin.segovia@intel.com>)

