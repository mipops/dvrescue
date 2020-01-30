AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = dvrescue
dvrescue_SOURCES = \
    ../../../Source/CLI/CLI_Help.cpp \
    ../../../Source/CLI/CLI_Main.cpp \
    ../../../Source/CLI/CommandLine_Parser.cpp \
    ../../../Source/Common/Core.cpp \
    ../../../Source/Common/Output.cpp \
    ../../../Source/Common/Output_Xml.cpp \
    ../../../Source/Common/Output_Webvtt.cpp \
    ../../../Source/Common/ProcessFile.cpp

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

TESTS = test/simple.sh test/xml.sh test/webvtt.sh test/framediscontinuity.sh

TESTING_DIR = test/TestingFiles

clone_testing_files: $(TESTING_DIR)

$(TESTING_DIR):
    git clone https://github.com/MediaArea/dvrescue-sample-data.git test/TestingFiles
