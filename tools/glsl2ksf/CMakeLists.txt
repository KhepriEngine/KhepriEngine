cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

project(glsl2ksf)

find_package(cxxopts REQUIRED)

add_executable(${PROJECT_NAME}
  src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    cxxopts::cxxopts
    Khepri
)
