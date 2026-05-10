    set(target logx)

    file(GLOB LOGX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

    set(LOGX_COMPILE_OPTIONS
        -Wall
        -Wextra
        -Werror=implicit-fallthrough
        -Werror=return-type
        -Werror=uninitialized
    )

    # --------- Shared library ----------
    add_library(${target} SHARED
        ${LOGX_SOURCES}
    )

    target_include_directories(${target} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_include_directories(${target} PRIVATE
        ${PROJECT_SOURCE_DIR}/deps/
        ${PROJECT_SOURCE_DIR}/deps/libyaml/include
    )

    target_link_libraries(${target} PRIVATE
        cjson
        yaml
    )

    target_compile_options(${target} PRIVATE ${LOGX_COMPILE_OPTIONS})

    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME ${target}
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    # ---------- Static Library ----------
    add_library(${target}_static STATIC
        ${LOGX_SOURCES}
    )

    target_include_directories(${target}_static PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_include_directories(${target}_static PRIVATE
        ${PROJECT_SOURCE_DIR}/deps/
        ${PROJECT_SOURCE_DIR}/deps/libyaml/include
    )


    target_link_libraries(${target}_static PRIVATE
        cjson
        yaml
    )

    target_compile_options(${target}_static PRIVATE ${LOGX_COMPILE_OPTIONS})

    set_target_properties(${target}_static PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        OUTPUT_NAME ${target}
    )

    # ------ Installation -------

    install(FILES ${LOGX_PUBLIC_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/logx
    )

    install(TARGETS ${target}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(TARGETS ${target}_static
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )