cmake_minimum_required(VERSION 3.29)

project(Sandbox)

set(SOURCES
    src/main.cpp
)

# set(CMAKE_BUILD_TYPE RelWithDebInfo)
# set(CMAKE_BUILD_TYPE Debug)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    mjoelnir::mjoelnir
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
    >
)