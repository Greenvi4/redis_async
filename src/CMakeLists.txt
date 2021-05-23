set(BOOST_COMPONENTS
    system
    thread
)
set(BOOST_VERSION 1.62)
find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

set(HEADERS
    ../include/redis_async/redis_async.hpp
        ../include/redis_async/common.hpp)

add_library(${PROJECT_NAME}
    ${HEADERS}
    redis_async.cpp
        common.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_SYSTEM_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

install_target_headers()
install_target_lib()