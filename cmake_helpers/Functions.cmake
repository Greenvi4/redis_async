macro(debug var)
    message("${var} = ${${var}}")
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
