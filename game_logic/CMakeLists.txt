cmake_minimum_required(VERSION 3.0.0)

project(Tetris_Game_Logic VERSION 0.1.0)

# add the executable
add_executable(Tetris_Game_Logic main.c)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)