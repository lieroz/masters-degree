cmake_minimum_required(VERSION 3.0)
project(benchmark)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O2 -ggdb")

add_executable(${PROJECT_NAME} main.cc thread.cc timer.cc)
target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} pthread m)
