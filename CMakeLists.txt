cmake_minimum_required(VERSION 3.25)
project(petite_truetype_renderer)

set(CMAKE_CXX_STANDARD 20)

find_package(Stb REQUIRED)

add_executable(petite_truetype_renderer main.cpp
        FontReader.cpp
        FontReader.h
        utils/Bit.cpp
        utils/Bit.h
        Glyph.cpp
        Glyph.h
        FrameBufferCanvas.cpp
        FrameBufferCanvas.h
        FrameBufferCanvas.h
        utils/Geometry.h
        utils/Debug.h
        utils/Unicode.h)

target_include_directories(petite_truetype_renderer PRIVATE ${Stb_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")


add_custom_command(TARGET petite_truetype_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts $<TARGET_FILE_DIR:petite_truetype_renderer>/fonts
)