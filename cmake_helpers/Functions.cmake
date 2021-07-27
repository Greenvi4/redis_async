macro(debug var)
    message("${var} = ${${var}}")
endmacro()

macro(declare_default_tests)
    build_gtest()
    file(GLOB_RECURSE tests_src
        *.cpp
        *.hpp
        *.h)
    add_executable(${PROJECT_NAME}Tests ${tests_src})
    target_link_libraries(${PROJECT_NAME}Tests
        PRIVATE
            ${PROJECT_NAME}
            "${GMOCK_BUILD_DIR}/gtest/libgtest.a"
            "${GMOCK_BUILD_DIR}/libgmock.a"
            pthread
        )

    install(TARGETS ${PROJECT_NAME}Tests
                RUNTIME
                    DESTINATION bin
                    COMPONENT tests
           )
endmacro()

macro(install_target_headers)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
            DESTINATION include/
            FILE_PERMISSIONS
                OWNER_READ OWNER_WRITE
                GROUP_READ
                WORLD_READ
            DIRECTORY_PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
            COMPONENT dev
            FILES_MATCHING
                PATTERN "*.h"
                PATTERN "*.hpp"
           )
endmacro()

macro(install_target_lib)
    install(TARGETS ${PROJECT_NAME}
                LIBRARY
                    DESTINATION lib/
                    PERMISSIONS
                        OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ
                    COMPONENT lib
                ARCHIVE
                    DESTINATION lib/
                    PERMISSIONS
                        OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ
                    COMPONENT lib
           )
endmacro()

macro(install_target_bin)
    install(TARGETS ${PROJECT_NAME}
                RUNTIME
                    DESTINATION bin
                    PERMISSIONS
                        OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE
                    COMPONENT bin
           )
endmacro()

macro(install_target_configs)
    install(FILES ${ARGN}
                DESTINATION share/
                PERMISSIONS
                    OWNER_READ OWNER_WRITE
                    GROUP_READ
                    WORLD_READ
                COMPONENT cfg
           )
endmacro()

function(build_gtest)
    set(DEFAULT_GMOCK_DIR "/usr/src/googletest/googlemock")
    set(DEFAULT_GMOCK_BUILD_DIR "${CMAKE_BINARY_DIR}/gmock")

    if(NOT GMOCK_DIR)
        set(GMOCK_DIR "${DEFAULT_GMOCK_DIR}")
    endif()
    if(NOT GMOCK_BUILD_DIR)
        set(GMOCK_BUILD_DIR "${DEFAULT_GMOCK_BUILD_DIR}")
    endif()

    if(NOT EXISTS "${GMOCK_BUILD_DIR}/gtest/libgtest.a" OR NOT EXISTS "${GMOCK_BUILD_DIR}/libgmock.a")
        message(STATUS "Could not found gMock libraries. Tring to build it myself")
        if(NOT EXISTS "${GMOCK_DIR}")
            message(FATAL_ERROR "You should install \"googletest\" package(sudo apt install googletest)")
        endif()
        file(MAKE_DIRECTORY "${GMOCK_BUILD_DIR}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} ${GMOCK_DIR}
            RESULT_VARIABLE result
            OUTPUT_VARIABLE capturedOutput
            ERROR_VARIABLE  capturedOutput
            WORKING_DIRECTORY "${GMOCK_BUILD_DIR}"
            )
        if(result)
            message(STATUS "${capturedOutput}")
            message(FATAL_ERROR "Build step for Google Mock failed: ${result}")
        endif()
        message(STATUS "Generating cmake for Google Mock done.")
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            OUTPUT_VARIABLE capturedOutput
            ERROR_VARIABLE  capturedOutput
            WORKING_DIRECTORY "${GMOCK_BUILD_DIR}"
            )
        if(result)
            message(STATUS "${capturedOutput}")
            message(FATAL_ERROR "Build step for Google Mock failed: ${result}")
        endif()
        message(STATUS "Building Google Mock done.")
    endif()
    set(GMOCK_DIR "${GMOCK_DIR}" PARENT_SCOPE)
    set(GMOCK_BUILD_DIR "${DEFAULT_GMOCK_BUILD_DIR}" PARENT_SCOPE)
endfunction()
