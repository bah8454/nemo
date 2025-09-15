# Required version for module support.
cmake_minimum_required(VERSION 3.28)
project(geigel)

# Set the C++ standard.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For the library.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add source files.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Add the library for the gdextension.
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Add compile options.
target_compile_definitions(${PROJECT_NAME} PRIVATE TYPED_METHOD_BIND)

############################################################################################

# Add godot-cpp.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/godot-cpp" build)
target_link_libraries(${PROJECT_NAME} PRIVATE godot::cpp)

############################################################################################

# Create a post-build command that copies the resulting dll to the correct folder.
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${PROJECT_NAME}>"
        "${CMAKE_SOURCE_DIR}/../bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Copying DLL to the bin directory"
)

#
# cmake .. -G Ninja
#
# ninja
#
