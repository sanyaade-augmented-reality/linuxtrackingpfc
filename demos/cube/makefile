INCLUDE = 
LIBDIR  = 

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lGL -lglut -ltpfcc

all: cube oldcube

cube : cube.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

oldcube : oldcube.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)

# to clean up:
clean:
	rm -f oldcube cube *.o *~

