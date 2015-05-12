cmake_minimum_required (VERSION 2.8)

project (kinskiGL)
CMAKE_POLICY( SET CMP0003 OLD )

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules
                      ${CMAKE_MODULE_PATH})
SET(GLFW_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/modules/glfw/include)

# Allow the developer to select if Dynamic or Static libraries are built
OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
OPTION (USE_CXX11 "Use C++11 language dialect and library" ON)

if(USE_CXX11)
    add_definitions(-DKINSKI_CPP11)
    if(APPLE)
    SET (CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    ELSE( APPLE )
    SET (CMAKE_CXX_FLAGS "-std=c++0x -pthread -Wall")
    endif(APPLE)
endif(USE_CXX11)

SET (LIB_TYPE SHARED)
IF (NOT BUILD_SHARED_LIBS)
  SET (LIB_TYPE STATIC)
ENDIF (NOT BUILD_SHARED_LIBS)

OPTION (BUILD_TESTS "Build Tests" ON)

##### BOOST
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_COMPONENTS system filesystem)
if(BUILD_TESTS)
    ENABLE_TESTING()
    set(BOOST_COMPONENTS ${BOOST_COMPONENTS} unit_test_framework)
endif(BUILD_TESTS)
find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})
#####

#glfw3
INCLUDE_DIRECTORIES(${GLFW_INCLUDE_DIRS})
#find_package(Xxf86vm REQUIRED)
set(LIBS ${LIBS} ${GLFW_LIBRARIES})

#link_directories("/usr/lib/x86_64-linux-gnu")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

ADD_SUBDIRECTORY("src")

