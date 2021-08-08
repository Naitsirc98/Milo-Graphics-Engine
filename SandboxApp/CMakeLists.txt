cmake_minimum_required(VERSION 3.1)

project("SandboxApp")

# Add source files
file(GLOB_RECURSE APPLICATION_SOURCE_FILES
        src/*.c
        src/*.cpp
        include/*.c
        include/*.cpp
        )

# Add header files
file(GLOB_RECURSE APPLICATION_HEADER_FILES
        src/*.h
        src/*.hpp
        include/*.h
        include/*.hpp
        )

add_executable(${PROJECT_NAME} ${APPLICATION_HEADER_FILES} ${APPLICATION_SOURCE_FILES})

message("Linking '${ENGINE_PROJECT_NAME}' to ${PROJECT_NAME}...")
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${ENGINE_PROJECT_DIR}/include)

target_link_libraries(${PROJECT_NAME} ${ENGINE_PROJECT_NAME})
set(RESOURCES_DIR ${ENGINE_PROJECT_DIR}/resources)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ENGINE_PROJECT_DIR}/resources/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)