cmake_minimum_required(VERSION 3.22)

project(Skylords_Reborn_Bot_API_Cpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -Wall")

add_executable(example_bot
    Example/Example.cpp
    Example/Main.cpp
)

find_package(Boost 1.74 REQUIRED)
target_link_libraries(example_bot LINK_PUBLIC ${Boost_LIBRARIES})
