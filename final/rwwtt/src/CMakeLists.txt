add_library(
    Application
    Application.hpp
    Application.cpp
    )

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

target_include_directories(Application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    Application
    ${GLFW_STATIC_LIBRARIES}
    Vulkan::Vulkan
    )
