# CMakeList.txt : CMake project for Milo Engine, built as a library, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

project("Milo")

set(CMAKE_CXX_STANDARD 17)

message("Running ${PROJECT_NAME} CMakeLists...")

# Find important dependencies before compiling
find_package(Vulkan REQUIRED)

message("Vulkan found ${Vulkan_INCLUDE_DIRS} ${Vulkan_LIBRARIES}")

# DEPENDENCIES
set(PROJECT_DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)

file(GLOB_RECURSE ENGINE_SOURCE_FILES
        src/*.cpp
        src/*.c
        dependencies/glad/*.c
        dependencies/stb/*.c
        )

file(GLOB_RECURSE ENGINE_HEADER_FILES
        include/*.h
        include/*.hpp
        )

file(GLOB_RECURSE IMGUI_SOURCE_FILES
        ${PROJECT_DEPENDENCIES_DIR}/imgui/*.cpp
        )

# Build this project as a library
# add_library(${PROJECT_NAME} STATIC ${ENGINE_HEADER_FILES} ${ENGINE_SOURCE_FILES})

# Build this project as an executable
add_executable(${PROJECT_NAME} ${ENGINE_INCLUDE_FILES} ${ENGINE_SOURCE_FILES} ${IMGUI_SOURCE_FILES} src/milo/scenes/components/Collider.cpp)

# CMAKE PROJECT DEPENDENCIES
message("Handling Milo Engine dependencies...")
add_subdirectory("${PROJECT_DEPENDENCIES_DIR}/glfw")
add_subdirectory(${PROJECT_DEPENDENCIES_DIR}/glm)
set(SHADERC_SKIP_TESTS ON)
add_subdirectory(${PROJECT_DEPENDENCIES_DIR}/shaderc)
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(${PROJECT_DEPENDENCIES_DIR}/assimp)

#set(BOOST_ROOT $ENV{BOOST_HOME})
#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_DEBUG FALSE)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost 1.77.0 REQUIRED COMPONENTS stacktrace)
#
#if(Boost_FOUND)
#    include_directories(${Boost_INCLUDE_DIRS})
#    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
#endif()

# INCLUDE DIRECTORIES
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/VulkanMemoryAllocator/include/)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/glad)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/KHR)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/glfw/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/glm)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/spdlog/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/imgui)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/stb)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/json)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/entt)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/shaderc/libshaderc/include)
target_include_directories(${PROJECT_NAME} PUBLIC $ENV{BOOST_HOME})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/tiny_obj_loader)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/assimp/include)

message("Boost = $ENV{BOOST_HOME}")

# PRECOMPILED HEADERS
#target_precompile_headers(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include/public_headers/engine/core/Common.h")

# Libraries
message("Linking libraries to Milo Engine...")
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
target_link_libraries(${PROJECT_NAME} shaderc)
target_link_libraries(${PROJECT_NAME} assimp)


message("Target directory is ${PROJECT_BINARY_DIR}")

set(RESOURCES_DIR ${PROJECT_SOURCE_DIR}/resources)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/resources/ ${PROJECT_BINARY_DIR}/resources)