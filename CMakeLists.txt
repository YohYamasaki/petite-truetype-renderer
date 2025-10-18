cmake_minimum_required(VERSION 3.25)
project(tiny_truetype_renderer)

set(CMAKE_CXX_STANDARD 20)

find_package(Stb REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(tiny_truetype_renderer main.cpp
        FontParser.cpp
        FontParser.h
        utils/Bit.h
        GlyphComponent.cpp
        GlyphComponent.h
        FrameBufferCanvas.cpp
        FrameBufferCanvas.h
        FrameBufferCanvas.h
        utils/Geometry.h
        utils/Debug.h
        utils/Unicode.h
        Glyph.cpp
        Glyph.h)

target_include_directories(tiny_truetype_renderer PRIVATE ${Stb_INCLUDE_DIR})
target_link_libraries(tiny_truetype_renderer PRIVATE glm::glm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")


add_custom_command(TARGET tiny_truetype_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts $<TARGET_FILE_DIR:tiny_truetype_renderer>/fonts
)