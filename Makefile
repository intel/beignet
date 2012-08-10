TOP=.
SUBDIRS=src src/sim src/intel src/x11

all::
	+cd backend && make all

clean::
	+cd backend && make clean

include $(TOP)/Makefile.shared
