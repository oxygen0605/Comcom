cmake_minimum_required(VERSION 3.10)

project (ComcomLib CXX)

file (GLOB LIB_CXX_CODE ./*.cpp)

add_library(${LIB_COMCOM} SHARED ${LIB_CXX_CODE})

#target_include_directories(
#    ${LIB_COMCOM} PRIVATE ${PROJECT_SOURCE_DIR}/../lib
#)

target_link_libraries(
    ${LIB_COMCOM}
    pthread
)

install (TARGETS
    ${LIB_COMCOM}
    LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/../exe
)