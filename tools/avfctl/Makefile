all: avfctl

clean:
		rm -f *.o avfctl DVRescue-Swift.h

# Build the Swift module and generate the Objective-C bridging header
DVRescue-Swift.h AvfCtl.o: ../../Source/Common/AvfCtl.swift
		swiftc -c -parse-as-library -emit-objc-header -emit-objc-header-path DVRescue-Swift.h \
			-module-name DVRescue -o AvfCtl.o ../../Source/Common/AvfCtl.swift

# Build the Swift CLI entry point
main.o: main.swift AvfCtl.o
		swiftc -c -I. -module-name avfctl -o main.o main.swift

# Link: Swift objects + system frameworks
avfctl: AvfCtl.o main.o
		swiftc -o $@ $^ \
			-Xlinker -framework -Xlinker IOKit \
			-Xlinker -framework -Xlinker Foundation \
			-Xlinker -framework -Xlinker AVFoundation \
			-Xlinker -framework -Xlinker CoreMedia \
			$(LDFLAGS)

# Legacy Objective-C build (kept for reference, use 'make avfctl-objc')
avfctl-objc: ../../Source/Common/AvfCtl-objc.o main-objc.o
		gcc -Wall -framework IOKit -framework Foundation -framework AVFoundation -framework CoreMedia -o $@ $(LDFLAGS) $^

%-objc.o: %.m
		gcc -Wall -I../../Source -c -fobjc-arc -I. -o $@ $(OBJCFLAGS) $<
