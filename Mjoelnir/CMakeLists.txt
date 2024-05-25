project(Mjoelnir)

set(SOURCES
    src/mjoelnir.c
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL  "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/cglm)

find_package(Vulkan REQUIRED)
include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIRS}
    /Users/singmyr/VulkanSDK/1.3.280.1/macOS/include
    ${PROJECT_SOURCE_DIR}/external/cglm/include
    ${PROJECT_SOURCE_DIR}/external/glfw/include
)

# add_compile_options(-Wall -Werror -Wpedantic)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
add_library(mjoelnir::mjoelnir ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})

# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()