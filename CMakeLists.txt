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
    src/io/container_stream.cpp
    src/io/file.cpp
    src/io/stream.cpp
    src/log/log.cpp
    src/math/matrix.cpp
    src/math/quaternion.cpp
    src/renderer/io/kmf.cpp
    src/renderer/model.cpp
    src/renderer/vulkan/renderer.cpp
    src/renderer/vulkan/render.cpp
    src/version_info.cpp
    src/version.cpp
)

set_property(
  SOURCE src/version.cpp
  APPEND
  PROPERTY COMPILE_DEFINITIONS
  KHEPRI_VERSION_MAJOR=${KHEPRI_VERSION_MAJOR}
  KHEPRI_VERSION_MINOR=${KHEPRI_VERSION_MINOR}
  KHEPRI_VERSION_PATCH=${KHEPRI_VERSION_PATCH}
  KHEPRI_VERSION_STRING="${KHEPRI_VERSION_MAJOR}.${KHEPRI_VERSION_MINOR}.${KHEPRI_VERSION_PATCH}"
  KHEPRI_VERSION_COMMIT="${KHEPRI_VERSION_COMMIT}"
  KHEPRI_VERSION_CLEAN=${KHEPRI_VERSION_CLEAN}
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

if(MSVC)
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
      _CRT_SECURE_NO_WARNINGS
  )
endif()

export(TARGETS ${PROJECT_NAME} NAMESPACE Khepri:: FILE KhepriTargets.cmake)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME})
install(DIRECTORY include/khepri DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ${PROJECT_NAME} DESTINATION lib)

add_subdirectory(tools)

#
# Tests
#
include(CTest)
if (BUILD_TESTING)
    find_package(GTest REQUIRED)

    include(GoogleTest)

    add_executable(${PROJECT_NAME}Tests
        tests/version_test.cpp
    )

    target_link_Libraries(${PROJECT_NAME}Tests
        PRIVATE
            ${PROJECT_NAME}
            GTest::gtest_main
    )

    gtest_discover_tests(${PROJECT_NAME}Tests)
endif()
