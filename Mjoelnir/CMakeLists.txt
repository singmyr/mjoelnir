project(Mjoelnir)

set(SOURCES
    src/mjoelnir.c
)

include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
add_library(mjoelnir::mjoelnir ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
