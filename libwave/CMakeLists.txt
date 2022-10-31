cmake_minimum_required(VERSION 3.0.0)
project(libwave)
include_directories(./ ../mlog)
add_library(wave STATIC wave.cpp)
