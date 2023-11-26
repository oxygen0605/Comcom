cmake_minimum_required(VERSION 3.10)

project (ComcomApp CXX)
set (MAIN_APP1 send)
set (MAIN_APP2 recv)

file (GLOB ALL_APP_CXX_CODE ./*.cpp)
file (GLOB APP1_CXX_CODE ./main_send.cpp)
file (GLOB APP2_CXX_CODE ./main_recv.cpp)

add_executable(
    ${MAIN_APP1}
    ${APP1_CXX_CODE}
)

add_executable(
    ${MAIN_APP2}
    ${APP2_CXX_CODE}
)

target_include_directories(
    ${MAIN_APP1}
    PRIVATE ${PROJECT_SOURCE_DIR}/../lib
)

target_include_directories(
    ${MAIN_APP2}
    PRIVATE ${PROJECT_SOURCE_DIR}/../lib
)

target_link_libraries(
    ${MAIN_APP1}
    #comcom
    ${LIB_COMCOM}
)

target_link_libraries(
    ${MAIN_APP2}
    ${LIB_COMCOM}
)

install (TARGETS
    ${MAIN_APP1}
    ${MAIN_APP2}
    RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../exe
)