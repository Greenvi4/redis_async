set(HEADERS
    ../include/redis_async/redis_async.hpp
)

add_library(${PROJECT_NAME}
    ${HEADERS}
    redis_async.cpp
    )

#target_link_libraries(${PROJECT_NAME} PUBLIC LibExchange Messages)
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

install_target_headers()
install_target_lib()