cmake_minimum_required(VERSION 3.10)
project(UltraFastMapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ultrafast_mapper
    src/main.cpp
    src/scheduler.cpp
    src/routing.cpp
    src/graph.cpp
    src/CGRA.cpp
    src/RESERVATIONtable.cpp
)

target_include_directories(ultrafast_mapper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(CTest)
if(BUILD_TESTING)
    add_executable(recurrence_tests
        tests/recurrence_tests.cpp
        src/scheduler.cpp
        src/routing.cpp
        src/graph.cpp
        src/CGRA.cpp
        src/RESERVATIONtable.cpp
    )
    target_include_directories(recurrence_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME recurrence_tests COMMAND recurrence_tests)
endif()
