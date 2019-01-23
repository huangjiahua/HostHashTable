cmake_minimum_required(VERSION 3.12)
project(HostHashTable)

set(CMAKE_CXX_STANDARD 11)

set(SRC_FILES ${CMAKE_SOURCE_DIR}/src/HashFunc.cpp ${CMAKE_SOURCE_DIR}/src/HostHashTable.cpp)

add_executable(HostHashTable main.cpp src/HostHashTable.cpp include/HostHashTable.h src/HashFunc.cpp src/HashFunc.h)

add_subdirectory(./test)