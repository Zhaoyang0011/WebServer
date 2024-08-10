cmake_minimum_required(VERSION 3.0)
project(WebServer CXX)


set(CMAKE_VERBOSE_MAKEFILE ON)

# 指定编译选项
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++20 -O0 -ggdb -Wall -Werror")

# -rdynamic: 将所有符号都加入到符号表中，便于使用dlopen或者backtrace追踪到符号
# -fPIC: 生成位置无关的代码，便于动态链接
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -fPIC")

# -Wno-unused-function: 不要警告未使用函数
# -Wno-builtin-macro-redefined: 不要警告内置宏重定义，用于重定义内置的__FILE__宏
# -Wno-deprecated: 不要警告过时的特性
# -Wno-deprecated-declarations: 不要警告使用带deprecated属性的变量，类型，函数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

include_directories(./src)

option(BUILD_TEST "ON for complile test" ON)

set(LIB_SRC
        src/util/CountDownLatch.cpp
        src/thread/Thread.cpp
        src/thread/ThreadPool.cpp
        src/thread/CurrentThread.cpp
        src/log/FixedBuffer.cpp
        src/log/AsyncLogger.cpp
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_library(WebServerLib STATIC ${LIB_SRC})

add_executable(Thread_test test/thread/Thread_test.cpp)
target_link_libraries(Thread_test WebServerLib)

add_executable(FixedBuffer_test test/log/FixedBuffer_test.cpp)
target_link_libraries(FixedBuffer_test WebServerLib)
