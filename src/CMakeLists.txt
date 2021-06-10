set(BOOST_COMPONENTS
        system
        thread
        )
set(BOOST_VERSION 1.62)
find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

set(HEADERS
        ../include/redis_async/redis_async.hpp
        ../include/redis_async/common.hpp
        ../include/redis_async/error.hpp
        ../include/redis_async/asio_config.hpp
        ../include/redis_async/future_config.hpp
        )

add_library(${PROJECT_NAME}
        ${HEADERS}
        redis_async.cpp
        common.cpp
        error.cpp
        details/base_connection.hpp
        details/base_connection.cpp
        details/events.hpp
        details/connection_fsm.hpp
        details/transport.hpp
        details/transport.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_SYSTEM_LIBRARIES} log4cxx)
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

install_target_headers()
install_target_lib()