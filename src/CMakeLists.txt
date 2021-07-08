set(BOOST_COMPONENTS
        system
        thread
        )
set(BOOST_VERSION 1.62)
find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

set(HEADERS
        ../include/redis_async/asio_config.hpp
        ../include/redis_async/command_options.hpp
        ../include/redis_async/commands.hpp
        ../include/redis_async/common.hpp
        ../include/redis_async/error.hpp
        ../include/redis_async/future_config.hpp
        ../include/redis_async/rd_types.hpp
        ../include/redis_async/redis_async.hpp

        ../include/redis_async/details/connection/base_connection.hpp
        ../include/redis_async/details/connection/concrete_connection.hpp
        ../include/redis_async/details/connection/connection_fsm.hpp
        ../include/redis_async/details/connection/connection_pool.hpp
        ../include/redis_async/details/connection/events.hpp
        ../include/redis_async/details/connection/handler_parse_result.hpp
        ../include/redis_async/details/connection/transport.hpp

        ../include/redis_async/details/protocol/command_args.hpp
        ../include/redis_async/details/protocol/markup_helper.hpp
        ../include/redis_async/details/protocol/parser.hpp
        ../include/redis_async/details/protocol/parser_types.hpp
        ../include/redis_async/details/protocol/serializer.hpp

        ../include/redis_async/details/redis_impl.hpp
        )

add_library(${PROJECT_NAME}
        ${HEADERS}
        common.cpp
        error.cpp
        redis_async.cpp
        commands.cpp

        details/connection/base_connection.cpp
        details/connection/connection_pool.cpp
        details/connection/transport.cpp

        details/redis_impl.cpp
        )

target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_SYSTEM_LIBRARIES} log4cxx)
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

install_target_headers()
install_target_lib()