##### Syphon
find_library(SYPHON NAMES SYPHON)
set(MODULE_INCLUDES "${SYPHON}/Headers" PARENT_SCOPE)
set(MODULE_LIBRARIES ${SYPHON} PARENT_SCOPE)
