cmake_minimum_required(VERSION 3.0.0)
project(mediaplayer)

add_subdirectory(alsa)
add_subdirectory(libwave)

include_directories(mlog alsa player libwave)

set(SRC_FILES player/player.cpp player/wavDecoder.cpp)
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} wave pthread)

add_executable(player main.cpp)
target_link_libraries(player ${PROJECT_NAME} alsaplay wave)
