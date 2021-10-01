# set project name and version
cmake_minimum_required(VERSION 3.15)
project(redis_async VERSION 1.0.0 LANGUAGES CXX)

get_directory_property(hasParent PARENT_DIRECTORY)

if(NOT hasParent)
    include("${CMAKE_SOURCE_DIR}/cmake_helpers/Functions.cmake")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    option(BUILD_TESTING "Enable building test" ON)
endif()

add_subdirectory(extern)
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
