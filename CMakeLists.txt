cmake_minimum_required(VERSION 3.23)
project(grk_initials C)

set(CMAKE_C_STANDARD 23)

add_executable(grk_initials main.c)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(grk_initials ${PNG_LIBRARY})
