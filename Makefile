TOP=.
SUBDIRS=src src/sim src/intel src/x11

all::

clean::
	+cd backend && make clean
	+cd utests && make clean

include $(TOP)/Makefile.shared

all::
	cd utests && make