cmake_minimum_required(VERSION 3.5)
project(BubbleJSON)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES test.cpp BubbleJSON/BubbleJson.cpp BubbleJSON/BubbleJson.h BubbleJSON/Struct.h BubbleJSON/BubbleValue.cpp BubbleJSON/BubbleValue.h test_file.cpp test_file.h)
add_executable(bubbleJson_test ${SOURCE_FILES})