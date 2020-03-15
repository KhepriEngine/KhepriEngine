cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

project(Khepri CXX)

# External libraries
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(gsl-lite REQUIRED)
find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME}
    src/application/application.cpp
    src/application/console_logger.cpp
    src/application/window.cpp
    src/log/log.cpp
    src/renderer/vulkan/renderer.cpp
    src/renderer/vulkan/render.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    fmt::fmt
    glfw::glfw
    gsl::gsl-lite
    Vulkan::Vulkan
)

export(TARGETS ${PROJECT_NAME} NAMESPACE Khepri:: FILE KhepriTargets.cmake)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME})
install(DIRECTORY include/khepri DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ${PROJECT_NAME} DESTINATION lib)
