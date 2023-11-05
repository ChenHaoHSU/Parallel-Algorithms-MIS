cmake_minimum_required(VERSION 3.13)
#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME "RubyMIS")
project(${PROJECT_NAME})


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# CXX flags
set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline ")
set(CMAKE_CXX_FLAGS_PROFILE "-O3 -pg -Winline")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

##################################################
#        Source Code                             #
##################################################
set(SRC
  src/main/main.cpp
  src/parser/parser.cpp
  src/solver/solver.cpp
)

set(SRC_EXE
  src/main/main.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(${PROJECT_NAME} ${SRC} ${SRC_EXE})

#target_link_libraries(
  #${PROJECT_NAME}
#)

#Install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX_DIR}/bin)
