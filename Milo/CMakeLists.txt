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

# Build this project as a library
# add_library(${PROJECT_NAME} STATIC ${ENGINE_HEADER_FILES} ${ENGINE_SOURCE_FILES} include/milo/core/MiloSubSystemManager.h src/milo/core/MiloSubSystemManager.cpp include/milo/common/Concurrency.h)

# Build this project as an executable
add_executable(${PROJECT_NAME} ${ENGINE_INCLUDE_FILES} ${ENGINE_SOURCE_FILES})

# DEPENDENCIES
set(PROJECT_DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)

# CMAKE PROJECT DEPENDENCIES
message("Handling Milo Engine dependencies...")
add_subdirectory("${PROJECT_DEPENDENCIES_DIR}/glfw")
add_subdirectory(${PROJECT_DEPENDENCIES_DIR}/glm)
add_subdirectory(${PROJECT_DEPENDENCIES_DIR}/spdlog)
set(SHADERC_SKIP_TESTS ON)
add_subdirectory(${PROJECT_DEPENDENCIES_DIR}/shaderc)

# INCLUDE DIRECTORIES
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/VulkanMemoryAllocator/include/)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/glad)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/KHR)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/glfw/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/glm)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/spdlog)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/imgui)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/stb)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/json/single_include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/entt)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/shaderc/libshaderc/include)

# PRECOMPILED HEADERS
#target_precompile_headers(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include/public_headers/engine/core/Common.h")

# Libraries
message("Linking libraries to Milo Engine...")
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} spdlog)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
target_link_libraries(${PROJECT_NAME} shaderc)

#target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/milo/common/Common.h)