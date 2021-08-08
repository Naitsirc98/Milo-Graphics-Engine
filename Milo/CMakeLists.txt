# CMakeList.txt : CMake project for Milo Engine, built as a library, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

project("Milo")

message("Running ${PROJECT_NAME} CMakeLists...")

# Find important dependencies before compiling
find_package(Vulkan REQUIRED)

# Add source files
if(${Vulkan_FOUND})
    set(VULKAN_SOURCE_FILES "src/*Vulkan*.cpp" "src/*Vulkan*.c" "dependencies/imgui/imgui_impl_vulkan.cpp")
    message("Vulkan found ${Vulkan_INCLUDE_DIRS} ${Vulkan_LIBRARIES}")
else()
    set(VULKAN_SOURCE_FILES "")
endif()

file(GLOB_RECURSE ENGINE_SOURCE_FILES
        src/*.cpp
        ${VULKAN_SOURCE_FILES}
        dependencies/glad/*.c
        dependencies/stb/*.c
        )

# Add header files
if(${Vulkan_FOUND})
    set(VULKAN_INCLUDE_FILES "dependencies/imgui/imgui_impl_vulkan.h")
else()
    set(VULKAN_INCLUDE_FILES "")
endif()

file(GLOB_RECURSE ENGINE_HEADER_FILES
        include/*.h
        ${VULKAN_INCLUDE_FILES}
        )

# Build this project as a library
add_library(${PROJECT_NAME} STATIC ${ENGINE_HEADER_FILES} ${ENGINE_SOURCE_FILES})

# DEPENDENCIES
set(PROJECT_DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)

if(${Vulkan_FOUND})
    target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/VulkanMemoryAllocator)
    target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE VULKAN)
endif()

# CMAKE PROJECT BASED DEPENDENCIES
message("Handling Milo Engine dependencies...")
add_subdirectory("${PROJECT_DEPENDENCIES_DIR}/glfw")
add_subdirectory(${PROJECT_DEPENDENCIES_DIR}/glm)
add_subdirectory(${PROJECT_DEPENDENCIES_DIR}/spdlog)

# INCLUDE DIRECTORIES
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/glad)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/KHR)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_DEPENDENCIES_DIR}/glfw/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/glm)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/spdlog)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/imgui)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/stb)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_DEPENDENCIES_DIR}/json/single_include)

# PRECOMPILED HEADERS
#target_precompile_headers(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include/public_headers/engine/core/Common.h")

# Libraries
message("Linking libraries to Milo Engine...")
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} spdlog)