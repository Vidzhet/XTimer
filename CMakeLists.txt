cmake_minimum_required(VERSION 3.21)

project(XTimer LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_CMake")

add_subdirectory(XTimer)
add_subdirectory(XTimerAPI)