all: avfctl

clean:
		rm *.o

avfctl: avfctl.o main.o
		gcc -Wall -framework IOKit -framework Foundation -framework AVFoundation -framework CoreMedia -o $@ $(LDFLAGS) $^

%.o: %.m
		gcc -Wall -c -fobjc-arc -I. -o $@ $(CFLAGS) $<
