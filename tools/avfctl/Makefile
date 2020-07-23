all: avfctl

clean:
		rm *.o

avfctl: avfctl.o main.o
		gcc -Wall -framework IOKit -framework Foundation -framework AVFoundation -framework CoreMedia -o $@ $^

%.o: %.m
		gcc -Wall -c -fobjc-arc -I. -o $@ $<


