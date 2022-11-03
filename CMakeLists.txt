cmake_minimum_required(VERSION 3.0.0)
project(mediaplayer)

add_subdirectory(alsa)
add_subdirectory(libwave)
add_subdirectory(libmad)

include_directories(mlog alsa player libwave libmad)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(SRC_FILES player/player.cpp player/wavDecoder.cpp player/mp3Decoder.cpp)
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} wave pthread mad)

add_executable(player main.cpp)
target_link_libraries(player ${PROJECT_NAME} alsaplay wave mad)
