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
        ../include/redis_async/rd_types.hpp
        ../include/redis_async/details/connection/base_connection.hpp
        ../include/redis_async/details/connection/events.hpp
        ../include/redis_async/details/connection/connection_fsm.hpp
        ../include/redis_async/details/connection/transport.hpp
        ../include/redis_async/details/connection/connection_pool.hpp
        ../include/redis_async/details/redis_impl.hpp
        ../include/redis_async/details/protocol/command.hpp
        ../include/redis_async/details/protocol/message.hpp
        )

add_library(${PROJECT_NAME}
        ${HEADERS}
        redis_async.cpp
        common.cpp
        error.cpp
        details/connection/transport.cpp
        details/connection/connection_pool.cpp
        details/connection/base_connection.cpp
        details/redis_impl.cpp
        details/protocol/message.cpp
        )

target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_SYSTEM_LIBRARIES} log4cxx)
target_include_directories(${PROJECT_NAME}
        PUBLIC ../include
        )

install_target_headers()
install_target_lib()