cmake_minimum_required(VERSION 3.16)

project(minesweeper VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

qt_add_executable(minesweeper
    src/main.cpp
    src/board.h
    src/board.cpp
    src/minesweeperwidget.h
    src/minesweeperwidget.cpp
)

target_link_libraries(minesweeper PRIVATE Qt6::Widgets)
