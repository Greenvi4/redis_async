# set project name and version
cmake_minimum_required(VERSION 3.7.2)
project(redis_async VERSION 2.2.0 LANGUAGES CXX)

get_directory_property(hasParent PARENT_DIRECTORY)

if(NOT hasParent)
    include("${CMAKE_SOURCE_DIR}/cmake_helpers/Functions.cmake")
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    option(BUILD_TESTING "Enable building test" ON)
endif()

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
