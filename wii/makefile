INCLUDE = -I/usr/include/
LIBDIR  = -L/usr/X11R6/lib 

COMPILERFLAGS = -Wall -O2
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lX11 -lXi -lXmu -lglut -lGL -lGLU -lm -lcwiid

all: wiitest

wiitest : wiitest.o
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $< $(LIBRARIES)  

# to clean up:
clean:
	rm wiitest *.o *.c~ 

