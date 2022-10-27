cmake_minimum_required(VERSION 3.0.0)
project(mediaplayer)

add_subdirectory(alsa)

include_directories(mlog alsa)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} alsaplay)
