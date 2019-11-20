add_executable(dvrescue
        CLI/CLI_Help.cpp
        CLI/CLI_Main.cpp
        CLI/CommandLine_Parser.cpp
        CLI/DVRescue.rc
        Common/Core.cpp
        Common/ProcessFile.cpp
        Common/XmlOutput.cpp
)
target_include_directories(dvrescue
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(dvrescue PRIVATE mediainfo)
set_target_properties(dvrescue PROPERTIES
        CXX_STANDARD 11)
install(TARGETS dvrescue)

