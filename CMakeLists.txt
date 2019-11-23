cmake_minimum_required(VERSION 3.10)
project(dvrescue
    DESCRIPTION
        "Data migration from DV tapes into digital files suitable for long-term preservation. Archivist-made software that supports data migration from DV tapes into digital files suitable for long-term preservation"
    HOMEPAGE_URL "https://MediaArea.net/DVRescue"
    VERSION 0.19.11
        LANGUAGES CXX
    )

find_package(ZenLib REQUIRED)
find_package(MediaInfoLib REQUIRED)

add_subdirectory(Source)
add_subdirectory(tools)

# Set the packaging variables in Project/CMake/CPackConfig.cmake. Anything
# not set here will be derived from the project metadata or use a default template,
include(Project/CMake/CPackConfig.cmake)
include(CPack)