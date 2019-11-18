project(dvrescue)

cmake_minimum_required(VERSION 2.8.11)

find_package(ZenLib REQUIRED)
find_package(MediaInfoLib REQUIRED)

set(dvrescue_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(dvrescue_SOURCES_PATH ${dvrescue_ROOT_PATH}/Source)

set(dvrescue_SRCS
  ${dvrescue_SOURCES_PATH}/CLI/CLI_Help.cpp
  ${dvrescue_SOURCES_PATH}/CLI/CLI_Main.cpp
  ${dvrescue_SOURCES_PATH}/CLI/CommandLine_Parser.cpp
  ${dvrescue_SOURCES_PATH}/CLI/DVRescue.rc
  ${dvrescue_SOURCES_PATH}/Common/Core.cpp
  ${dvrescue_SOURCES_PATH}/Common/ProcessFile.cpp
  ${dvrescue_SOURCES_PATH}/Common/XmlOutput.cpp
)

set(dvrescue_TOOLS
  ${dvrescue_ROOT_PATH}/tools/dvloupe
  ${dvrescue_ROOT_PATH}/tools/dvmap
  ${dvrescue_ROOT_PATH}/tools/dvpackager
  ${dvrescue_ROOT_PATH}/tools/dvplay
  ${dvrescue_ROOT_PATH}/tools/dvsampler
)

add_executable(dvrescue ${dvrescue_SRCS})
target_include_directories(dvrescue PRIVATE ${dvrescue_SOURCES_PATH})
target_link_libraries(dvrescue ${ZenLib_LIBRARY} ${MediaInfoLib_LIBRARY})

install(TARGETS dvrescue DESTINATION bin)
if(UNIX)
  install(PROGRAMS ${dvrescue_TOOLS} DESTINATION bin)
endif()

if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
  )
  add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  )
endif()
