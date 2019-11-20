cmake_minimum_required(VERSION 3.10)
project(dvrescue)

find_package(ZenLib REQUIRED)
find_package(MediaInfoLib REQUIRED)

add_subdirectory(Source)
add_subdirectory(tools)
