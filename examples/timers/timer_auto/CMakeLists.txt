set(TARGET timer_auto)

add_compile_options(
    -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=
)

add_executable(${TARGET}
    main.c
)

target_link_libraries(${TARGET}
    logx
)