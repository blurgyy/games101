cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

project(rwwtt)
add_executable(rwwtt main.cpp)

add_subdirectory(src)

target_link_libraries(
    rwwtt
    Application
    )
