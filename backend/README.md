Beignet Compiler
================

This code base contains the compiler part of the Beignet OpenCL stack. The
compiler is responsible to take a OpenCL language string and to compile it into
a binary that can be executed on Intel integrated GPUs.

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

Ben Segovia (<benjamin.segovia@intel.com>)

