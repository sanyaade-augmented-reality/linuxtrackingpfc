INCLUDE = 
LIBDIR  = 

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lGL -lglut -ltpfcc -lquat

all: trackIO

trackIO : trackIO.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

# to clean up:
clean:
	rm -f trackIO *.o *~

