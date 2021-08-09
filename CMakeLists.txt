cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project("Milo-Graphics-Engine")

set(CMAKE_CXX_STANDARD 17)

set(ENGINE_PROJECT_DIR ${CMAKE_SOURCE_DIR}/Milo)
set(APPLICATION_PROJECT_DIR ${CMAKE_SOURCE_DIR}/SandboxApp)
set(ENGINE_PROJECT_NAME "Milo")
set(APPLICATION_PROJECT_NAME "SandboxApp")

# Include sub-projects.
add_subdirectory("Milo")
#add_subdirectory("SandboxApp")