cmake_minimum_required(VERSION 3.0.0)
project(libmad)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCONFIG_64BIT -DFPM_64BIT -DHAVE_CONFIG_H -DASO_INTERLEAVE2 -DASO_ZEROCHECK -Os -fomit-frame-pointer")
set(SRC_FILES   libmad-0.15.1b/bit.c
                libmad-0.15.1b/decoder.c
                libmad-0.15.1b/fixed.c
                libmad-0.15.1b/frame.c
                libmad-0.15.1b/huffman.c
                libmad-0.15.1b/layer12.c
                libmad-0.15.1b/layer3.c
                libmad-0.15.1b/stream.c
                libmad-0.15.1b/timer.c
                libmad-0.15.1b/synth.c
                libmad-0.15.1b/version.c )
include_directories(libmad-0.15.1b)
add_library(mad STATIC ${SRC_FILES})

