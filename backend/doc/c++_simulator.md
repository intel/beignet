C++ Simulator
=============

The compiler can output both Gen ISA or C++ code that can be run directly on
CPUs. The C++ simulator (which is technically run and owned by the run-time, not
the compiler) basically loads a shared object, get the kernel code from it and
run it. 

The role of the compiler is to generate this shared object.

Most of the code related to the C++ simulator can be found in:
`src/backend/sim_context.*pp`

`src/backend/sim_program.*pp`

`src/backend/sim_program.h`

`src/backend/sim/sim_vector.*pp`

This files are responsible for building the C++ string for the kernel and to
compile a shared object from it. Once done and through the C API specified by
`backend/program.h`, the run-time can load and run the compiled code very
similarly to how the Gen kernel itself.

Files in `backend/sim` contains the actual C++ code used when compiled the
GenIR. They are basically a set of C++ templates and macros whose the goal is to
make the kernel code generation really easy.

[Up](../README.html)

