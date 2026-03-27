AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = dvrescue
dvrescue_SOURCES = \
    ../../../Source/CLI/CLI_Help.cpp \
    ../../../Source/CLI/CLI_Main.cpp \
    ../../../Source/CLI/CommandLine_Parser.cpp \
    ../../../Source/Common/Core.cpp \
    ../../../Source/Common/Merge.cpp \
    ../../../Source/Common/MergeTs.cpp \
    ../../../Source/Common/Output.cpp \
    ../../../Source/Common/Output_Captions_Decode.cpp \
    ../../../Source/Common/Output_Captions_Scc.cpp \
    ../../../Source/Common/Output_Xml.cpp \
    ../../../Source/Common/Output_Webvtt.cpp \
    ../../../Source/Common/ProcessFile.cpp \
    ../../../Source/Common/ProcessFileWrapper.cpp \
    ../../../Source/Common/SimulatorWrapper.cpp \
    ../../../Source/ThirdParty/ccdecoder/ccdecoder_internal.c \
    ../../../Source/ThirdParty/ccdecoder/ccdecoder_line21.c \
    ../../../Source/ThirdParty/ccdecoder/ccdecoder_line21field.c \
    ../../../Source/ThirdParty/ccdecoder/ccdecoder_onscreen.c \
    ../../../Source/ThirdParty/ccdecoder/ccdecoder_subrip.c \
    ../../../Source/ThirdParty/TimeCode/TimeCode.cpp

INCLUDES = -I../../../Source \
           -I../../../Source/ThirdParty/ccdecoder \
           -I../../../Source/ThirdParty/TimeCode \
           -I../../../Source/ThirdParty/serial/include \
           -I../../../../MediaInfoLib/Source \
           -I../../../../ZenLib/Source

AM_CXXFLAGS = -std=c++17

bin_SCRIPTS = \
    ../../../tools/dvloupe \
    ../../../tools/dvmap \
    ../../../tools/dvpackager \
    ../../../tools/dvplay \
    ../../../tools/dvsampler

if BUILD_AVFCTL
    # Swift source compiled to object file; Obj-C++ wrapper bridges Swift to C++
    SWIFT_MODULE_NAME = DVRescue
    SWIFT_HEADER = DVRescue-Swift.h
    SWIFT_SRC = ../../../Source/Common/AvfCtl.swift
    SWIFT_OBJ = AvfCtl_swift.o

    $(SWIFT_OBJ) $(SWIFT_HEADER): $(SWIFT_SRC)
	swiftc -c -parse-as-library -emit-objc-header \
		-emit-objc-header-path $(SWIFT_HEADER) \
		-module-name $(SWIFT_MODULE_NAME) -o $(SWIFT_OBJ) $(SWIFT_SRC)

    dvrescue_SOURCES += \
        ../../../Source/Common/AvfCtlWrapper.mm
    dvrescue_LDADD = $(SWIFT_OBJ)
    dvrescue_LDFLAGS = -framework IOKit -framework Foundation -framework AVFoundation -framework CoreMedia
    BUILT_SOURCES = $(SWIFT_HEADER)

    AVFCTL = ../../../tools/avfctl/avfctl
    $(AVFCTL):
	$(MAKE) -C ../../../tools/avfctl LDFLAGS="$(LDFLAGS)"
    all: $(AVFCTL)

    bin_SCRIPTS += ../../../tools/avfctl/avfctl
endif

if BUILD_DECKLINK
    dvrescue_SOURCES += ../../../Source/Common/DecklinkWrapper.cpp \
                        ../../../Source/Common/Output_Mkv.cpp \
                        ../../../Source/Common/SignalStats.cpp
endif

if BUILD_DECKLINK

else
if BUILD_SIMULATOR
    dvrescue_SOURCES += ../../../Source/Common/Output_Mkv.cpp \
                        ../../../Source/Common/SignalStats.cpp
endif
endif

if BUILD_SONY9PIN
    dvrescue_SOURCES += ../../../Source/Common/Sony9PinWrapper.cpp \
                        ../../../Source/ThirdParty/serial/src/impl/list_ports/list_ports_osx.cc \
                        ../../../Source/ThirdParty/serial/src/impl/unix.cc \
                        ../../../Source/ThirdParty/serial/src/serial.cc
endif

if BUILD_LNX1394
    dvrescue_SOURCES += ../../../Source/Common/LinuxWrapper.cpp
endif

# man1_MANS = ../../../Source/CLI/dvrescue.1

AM_TESTS_FD_REDIRECT = 9>&2

TESTS = test/simple.sh test/xml.sh test/webvtt.sh test/captions.sh test/framediscontinuity.sh test/tools.sh

TESTING_DIR = test/TestingFiles

clone_testing_files: $(TESTING_DIR)

$(TESTING_DIR):
	git clone https://github.com/MediaArea/dvrescue-sample-data.git test/TestingFiles
