# 设置cmake的最低版本和项目名称
cmake_minimum_required(VERSION 2.8)
project(MPRPC)

set(CMAKE_BUILD_TYPE "Debug")

#设置项目可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置项目库文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
#设置项目文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src包含mprpc框架所有的相关代码
add_subdirectory(src)
# example包含了mprpc框架使用的代码示例
add_subdirectory(example)