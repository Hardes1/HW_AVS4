cmake_minimum_required(VERSION 3.21)
project (HW_AVS4)

set_target_properties(PROPERTIES OUTPUT_NAME "main.out")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_options(-lpthread)
add_executable(HW_AVS4 main.cpp)
set_property(TARGET HW_AVS4 PROPERTY OUTPUT_NAME "main.out")
