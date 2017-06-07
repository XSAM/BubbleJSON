cmake_minimum_required(VERSION 2.8)
project(BubbleJSON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES test.cpp BubbleJSON/BubbleJson.cpp BubbleJSON/BubbleJson.h BubbleJSON/Struct.h BubbleJSON/BubbleValue.cpp BubbleJSON/BubbleValue.h test_file.cpp test_file.h)
add_executable(bubbleJson_test ${SOURCE_FILES})