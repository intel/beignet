clang -x cl -emit-llvm -O3 -ccc-host-triple ptx32 -c $1.cl -o $1.o
llvm-dis $1.o
mv $1.o.ll $1.ll

