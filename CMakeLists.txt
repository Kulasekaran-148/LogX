cmake_minimum_required(VERSION 3.16)
project(LogX VERSION 1.0 LANGUAGES C)

add_library(logx src/logx.c src/log_config.c src/log_utils.c)
target_include_directories(logx PUBLIC include)

# Optional: install support
install(TARGETS logx DESTINATION lib)
install(DIRECTORY include/logx DESTINATION include)
