file(GLOB_RECURSE tests_src
    *.cpp
    *.hpp
    *.h)
add_executable(${PROJECT_NAME}_test ${tests_src})
target_link_libraries(${PROJECT_NAME}_test
    PRIVATE
        ${PROJECT_NAME}
        gmock_main
        pthread
    )
add_test(unit_tests ${PROJECT_NAME}_test)
install(TARGETS ${PROJECT_NAME}_test
            RUNTIME
                DESTINATION bin
                COMPONENT tests
       )
