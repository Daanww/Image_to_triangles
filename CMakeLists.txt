cmake_minimum_required(VERSION 3.0.0)
project(Image_triangles VERSION 0.1.0)

include(CTest)
enable_testing()

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")


find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(Image_triangles main.c image_data.h image_data.c triangle_data.h triangle_data.c)
target_link_libraries(Image_triangles ${SDL2_LIBRARIES} SDL2_image m SDL2_gfx)

target_compile_options(Image_triangles PUBLIC -pedantic -Wall -O2)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
