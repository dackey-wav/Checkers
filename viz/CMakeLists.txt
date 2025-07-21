cmake_minimum_required(VERSION 3.16)
project(CheckersViz)

set(CMAKE_CXX_STANDARD 17)

set(SFML_DIR "C:/Libraries/SFML-3.0.0/lib/cmake/SFML")

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(viz
    ../viz/viz.cpp
    ../src/Board.cpp
    ../src/Piece.cpp
    ../src/Tile.cpp
    ../src/Move.cpp
)
set_target_properties(viz PROPERTIES WIN32_EXECUTABLE OFF)
target_link_libraries(viz
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)