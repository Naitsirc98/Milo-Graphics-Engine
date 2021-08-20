cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project("Milo-Graphics-Engine")

set(CMAKE_CXX_STANDARD 17)

set(ENGINE_PROJECT_DIR ${CMAKE_SOURCE_DIR}/Milo)
set(ENGINE_PROJECT_NAME "Milo")

# Include sub-projects.
add_subdirectory("Milo")