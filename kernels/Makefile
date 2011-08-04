GEN6 := $(patsubst %.cl,%.gen6.bin,$(wildcard *.cl))
GEN7 := $(patsubst %.cl,%.gen7.bin,$(wildcard *.cl))

%.gen6.bin: %.cl
	wine ../bin/TC_Tester.exe -f $< -dev gen6 -in cl_text -out dev_bin

%.gen7.bin: %.cl
	wine ../bin/TC_Tester.exe -f $< -dev gen7 -in cl_text -out dev_bin

gen6: $(GEN6)
	mv *bin ./gen6
	rm -rf *ghal3d *ll *krn

gen7: $(GEN7)
	mv *bin ./gen7
	rm -rf *ghal3d *ll *krn

