INCLUDE = 
LIBDIR  = 

#COMPILERFLAGS = -Wall -O2
COMPILERFLAGS = -O2
CC = g++
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -lvrpnserver -lvrpnatmel -lpthread  -lquat -lglut -lGL

all: libtpfcc.so.1.0.1

tpfcc.o : TrackingPFC_client.C TrackingPFC_client.h
	$(CC) $(CFLAGS) -shared -fpic -c -o $@ $(LIBDIR) $< $(LIBRARIES)

libtpfcc.so.1.0.1 : tpfcc.o
	gcc $(CFLAGS) -shared -Wl,-soname,libtpfcc.so.1 -o $@ $(LIBDIR) $< -lc $(LIBRARIES)

install : libtpfcc.so.1.0.1
	cp libtpfcc.so.1.0.1 /usr/lib/
	ln -s -f /usr/lib/libtpfcc.so.1.0.1 /usr/lib/libtpfcc.so.1
	ln -s -f /usr/lib/libtpfcc.so.1.0.1 /usr/lib/libtpfcc.so
	cp TrackingPFC_client.h /usr/include/

test : test.C
	$(CC) $(CFLAGS) -o $@ $(LIBDIR) $^ $(LIBRARIES) -ltpfcc


# to clean up:
clean:
	rm *~ test *.o *.so*

