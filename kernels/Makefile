GEN6  := $(patsubst %.cl,%.gen6.bin,$(wildcard *.cl))
GEN7  := $(patsubst %.cl,%.gen7.bin,$(wildcard *.cl))
GEN75 := $(patsubst %.cl,%.gen75.bin,$(wildcard *.cl))

%.gen6.bin: %.cl
	wine ../bin/gen6/TC_Tester.exe -f $< -dev gen6 -in cl_text -out dev_bin

%.gen7.bin: %.cl
	wine ../bin/TC_Tester.exe -f $< -dev gen7 -in cl_text -out dev_bin

%.gen75.bin: %.cl
	wine ../bin/TC_Tester.exe -f $< -dev gen75 -in cl_text -out dev_bin

clean75:
	rm -rf ./gen75/*bin

clean7:
	rm -rf ./gen7/*bin

clean6:
	rm -rf ./gen6/*bin

gen6: $(GEN6)
	mv *bin ./gen6
	rm -rf *ghal3d *ll *krn

gen7: $(GEN7) clean7
	mv *bin ./gen7
	rm -rf *ghal3d *ll *krn *krn7 *_0.cl

gen75:  $(GEN75) clean75
	mv *bin ./gen75
	rm -rf *ghal3d *ll *krn *krn75 *_0.cl

