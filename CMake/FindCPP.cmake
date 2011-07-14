#
# Try to find c preprocessor
# Once done this will define
#
# CPP_FOUND
# CPP_PATH
#

FIND_PROGRAM(CPP_PATH cpp
  /bin/
  /usr/bin/
  /usr/local/bin
  ~/bin
  /opt/local/bin/
  DOC "The directory where cpp is")

IF(CPP_PATH)
  SET(CPP_FOUND 1 CACHE STRING "Set to 1 if CPP is found, 0 otherwise")
ELSE(CPP_PATH)
  SET(CPP_FOUND 0 CACHE STRING "Set to 1 if CPP is found, 0 otherwise")
ENDIF(CPP_PATH)

MARK_AS_ADVANCED(CPP_FOUND)


