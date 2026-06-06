cmake_minimum_required(VERSION 3.15)
project(limit_order_book_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES
    src/main.cpp
    src/Order.cpp
    src/OrderBook.cpp
    src/HttpServer.cpp
    src/Trade.cpp
)

set(TEST_SOURCE_FILES
    tests/tests.cpp
    src/Order.cpp
    src/OrderBook.cpp
)

add_executable(orderbook ${SOURCE_FILES})
target_include_directories(orderbook PRIVATE include)
if (WIN32)
    target_link_libraries(orderbook PRIVATE ws2_32)
endif()

add_executable(tests ${TEST_SOURCE_FILES})
target_include_directories(tests PRIVATE include)

enable_testing()
add_test(NAME orderbook_tests COMMAND tests)
