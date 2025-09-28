cmake_minimum_required(VERSION 3.30)
project(petite_truetype_renderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(petite_truetype_renderer main.cpp
        FontReader.cpp
        FontReader.h)

add_custom_command(TARGET petite_truetype_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts $<TARGET_FILE_DIR:petite_truetype_renderer>/fonts
)