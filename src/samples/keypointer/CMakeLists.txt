##### OPENCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "-------LIBS for keypointer: ${LIBS}")
set(LIBS ${LIBS} ${OpenCV_LIBS})
#####

set(LIBS ${LIBS} "cv")

KINSKI_ADD_SAMPLE(${itemName} ${folderItem})

