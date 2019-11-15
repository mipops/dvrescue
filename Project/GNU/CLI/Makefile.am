AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = dvrescue
dvrescue_SOURCES = \
    ../../../Source/CLI/CLI_Help.cpp \
    ../../../Source/CLI/CLI_Main.cpp \
    ../../../Source/CLI/CommandLine_Parser.cpp \
    ../../../Source/Common/Core.cpp \
    ../../../Source/Common/ProcessFile.cpp \
    ../../../Source/Common/XmlOutput.cpp

INCLUDES = -I../../../Source \
           -I../../../../MediaInfoLib/Source \
           -I../../../../ZenLib/Source
AM_CXXFLAGS = -std=c++11

bin_SCRIPTS = \
    ../../../tools/dvloupe \
    ../../../tools/dvmap \
    ../../../tools/dvpackager \
    ../../../tools/dvplay \
    ../../../tools/dvsampler

# man1_MANS = ../../../Source/CLI/dvrescue.1

AM_TESTS_FD_REDIRECT = 9>&2
