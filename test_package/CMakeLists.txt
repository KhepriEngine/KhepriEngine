cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

project(test_package LANGUAGES CXX)

find_package(Khepri REQUIRED CONFIG)

add_executable(${PROJECT_NAME} test_package.cpp)
target_link_libraries(${PROJECT_NAME} khepri::khepri)