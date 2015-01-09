# Find the standalone gbe compiler
#
# STANDALONE_GBE_COMPILER_DIR - base path of standalone compiler
# STANDALONE_GBE_COMPILER     - full file name of standalone compiler
# GEN_PCI_ID                  - Gen's PCI ID

IF (STANDALONE_GBE_COMPILER_DIR)
  FIND_PROGRAM(STANDALONE_GBE_COMPILER
             NAMES gbe_bin_generater
             DOC "standalone gbe compiler executable"
             PATHS ${STANDALONE_GBE_COMPILER_DIR} NO_DEFAULT_PATH)
ELSE (STANDALONE_GBE_COMPILER_DIR)
  FIND_PROGRAM(STANDALONE_GBE_COMPILER
             NAMES gbe_bin_generater
             DOC "standalone gbe compiler executable"
             PATHS /usr/local/lib/beignet/)
ENDIF (STANDALONE_GBE_COMPILER_DIR)

IF (STANDALONE_GBE_COMPILER)
  MESSAGE(STATUS "Looking for standalone gbe compiler - found at ${STANDALONE_GBE_COMPILER}")
  STRING(REGEX REPLACE "(.*)/.*" "\\1" STANDALONE_GBE_COMPILER_DIR ${STANDALONE_GBE_COMPILER})
  IF (NOT GEN_PCI_ID)
    Find_Program(LSPCI lspci)
    IF (LSPCI)
      MESSAGE(STATUS "Looking for lspci - found")
    ELSE (LSPCI)
      MESSAGE(FATAL_ERROR "Looking for lspci - not found")
    ENDIF (LSPCI)
    EXECUTE_PROCESS(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/GetGenID.sh"
             OUTPUT_VARIABLE GEN_PCI_ID)
    MESSAGE(STATUS "Platform Gen PCI id is " ${GEN_PCI_ID})
  ENDIF (NOT GEN_PCI_ID)
ELSE (STANDALONE_GBE_COMPILER)
  MESSAGE(FATAL_ERROR "Looking for standalone gbe compiler - not found")
ENDIF (STANDALONE_GBE_COMPILER)