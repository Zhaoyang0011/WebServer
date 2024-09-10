cmake_minimum_required(VERSION 3.0)
project(WebServer CXX ASM)


set(CMAKE_VERBOSE_MAKEFILE ON)

# 指定编译选项

# -Wno-unused-function: 不要警告未使用函数
# -Wno-builtin-macro-redefined: 不要警告内置宏重定义，用于重定义内置的__FILE__宏
# -Wno-deprecated: 不要警告过时的特性
# -Wno-deprecated-declarations: 不要警告使用带deprecated属性的变量，类型，函数
# -rdynamic: 将所有符号都加入到符号表中，便于使用dlopen或者backtrace追踪到符号
# -fPIC: 生成位置无关的代码，便于动态链接
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c++20 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c20 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(./src)

option(BUILD_TEST "ON for complile test" ON)

set(LIB_SRC
        src/base/Timestamp.cpp
        src/util/CountDownLatch.cpp
        src/thread/Thread.cpp
        src/thread/ThreadPool.cpp
        src/thread/CurrentThread.cpp
        src/coroutine/fcontext/fcontext.S
        src/coroutine/Coroutine.cpp
        #        src/coroutine/CoroutineScheduler.cpp
        src/log/Logger.cpp
        src/log/FixedBuffer.cpp
        src/log/AsyncLogger.cpp
        src/poller/Poller.cpp
        src/poller/DefaultPoller.cpp
        src/poller/EpollPoller.cpp
        src/Channel.cpp
        src/EventLoop.cpp
        src/EventLoopThread.cpp
        src/EventLoopThreadPool.cpp
        src/Timer.cpp
        src/TimerQueue.cpp
        test/coroutine_test.cpp
        src/kv/SkipList.cpp
        #        src/Socket.cpp
        #        src/InternetAddress.cpp
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_library(WebServerLib STATIC ${LIB_SRC})

add_executable(Thread_test test/thread/Thread_test.cpp)
target_link_libraries(Thread_test WebServerLib)

add_executable(FixedBuffer_test test/log/FixedBuffer_test.cpp)
target_link_libraries(FixedBuffer_test WebServerLib)

add_executable(TimerQueue_test test/TimerQueue_test.cpp)
target_link_libraries(TimerQueue_test WebServerLib)

add_executable(EventLoopThread_test test/EventLoopThread_test.cpp)
target_link_libraries(EventLoopThread_test WebServerLib)

add_executable(EventLoop_test1 test/event_loop/test1.cpp)
target_link_libraries(EventLoop_test1 WebServerLib)

add_executable(Coroutine_test test/coroutine_test.cpp)
target_link_libraries(Coroutine_test WebServerLib)

#add_executable(raw_epoll_echo_test test/raw_epoll_echo_test.cpp)
#target_link_libraries(raw_epoll_echo_test WebServerLib)
