cmake_minimum_required(VERSION 3.5)
project(BubbleJSON)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES test.cpp BubbleJson.cpp BubbleJson.h Struct.h BubbleValue.cpp BubbleValue.h BubbleMember.h)
add_executable(BubbleJSON ${SOURCE_FILES})