project(Mjoelnir
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    include/mjoelnir.hpp
    src/mjoelnir.cpp
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL  "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

find_package(Vulkan REQUIRED)
include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/external/glm/include
    ${PROJECT_SOURCE_DIR}/external/glfw/include
)

# add_compile_options(-Wall -Werror -Wpedantic)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
add_library(mjoelnir::mjoelnir ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})

get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()