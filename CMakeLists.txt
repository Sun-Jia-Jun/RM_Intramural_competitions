cmake_minimum_required(VERSION 3.10)
project(校内赛)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(main main.cpp)
add_executable(test test.cpp)
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(test ${OpenCV_LIBS})
