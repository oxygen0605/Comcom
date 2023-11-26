cmake_minimum_required (VERSION 3.10)

project (ServiceOrientedApplicationFramework)

# User Define
set (_DefaultEnableDocumentation off)
set (_DefaultBuildType "Debug")
set (_InstallPrefix ${PROJECT_SOURCE_DIR})
set (_ProjectVersion "0.0.1")

# System Define
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set (CMAKE_BUILD_TYPE ${_DefaultBuildType})
endif ()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${_InstallPrefix}" CACHE PATH "..." FORCE)
endif ()

set (PROJECT_VERSION ${_ProjectVersion})

## Test Project ON/OFF
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set (ENABLE_UNIT_TEST OFF)
else ()
    set (ENABLE_UNIT_TEST OFF)
endif ()

## Documentation Project ON/OFF
option (ENABLE_DOCUMENTATION "Create doxygen docs" ${_DefaultEnableDocumentation})


## App Setting
set (LIB_COMCOM comcom)
# set (LIB_NAME HWLib)
set (DOC_NAME Sample)
set (TEST_APP_NAME SampleTest)

set(CMAKE_DEBUG_POSTFIX "d")

#Custom Command
ADD_CUSTOM_TARGET(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug -DENABLE_UNIT_TEST=OFF ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
)

ADD_CUSTOM_TARGET(cmakeclean
    COMMAND echo "rm -rf CMakeFiles CMakeCache.txt Makefile *.cmake"
    COMMAND rm -rf CMakeFiles CMakeCache.txt Makefile *.cmake
    COMMENT "cmake clean ..."
)

# print message
message (STATUS "[INFO]: CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")

# compile options
#set (CMAKE_C_COMPILER:FILEPATH /usr/bin/gcc)
set (CMAKE_C_FLAGS "-Wall")
set (CMAKE_C_FLAGS_DEBUG "-g3 -O0 ")
set (CMAKE_C_FLAGS_RELEASE "-O2 -s -DNEBUG")

#set (CMAKE_CXX_COMPILER:FILEPATH /usr/bin/g++)
set (CMAKE_CXX_FLAGS "-std=c++14 -Wall")
set (CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -s -DNEBUG")

add_subdirectory(app)
add_subdirectory(lib)
#add_subdirectory(ServiceApp/cpp)
#add_subdirectory(ServiceClient)
#add_subdirectory(ServiceProbe)
